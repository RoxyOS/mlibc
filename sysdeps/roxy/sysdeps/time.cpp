#include <errno.h>
#include <mlibc/all-sysdeps.hpp>
#include <pthread.h>
#include <roxy/syscall.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "errors.hpp"

namespace mlibc {

namespace {

// The realtime signal used to deliver a `SIGEV_THREAD` expiration to its helper thread. Roxy
// reserves this number for this mechanism; the helper thread alone blocks it, and the kernel
// delivers the timer's `SIGEV_THREAD_ID` signal directly to that thread's pending queue.
constexpr int roxyTimerSignal = SIGRTMIN;

// A Roxy timer handle at the userland boundary. Every `timer_create` returns a pointer to one of
// these; the kernel-side identity lives in `kernel_id`.
struct roxyTimerHandle {
	int32_t kernel_id;
	int32_t notifyMode; // SIGEV_NONE / SIGEV_SIGNAL / SIGEV_THREAD
	struct sigevent notify;
	// SIGEV_THREAD bookkeeping:
	int32_t helperTid; // helper's kernel tid, published by the helper
	int32_t armed;     // futex handshake; 1 once the kernel timer exists
	int32_t quit;      // 1 -> the helper thread exits
	pthread_t helper;  // the helper thread (SIGEV_THREAD only)
};

// The helper thread for a SIGEV_THREAD timer: blocks the internal signal in itself, publishes its
// kernel tid, then loops on sigtimedwait invoking the user callback on each expiration.
void *roxyTimerHelper(void *arg) {
	auto *h = static_cast<roxyTimerHandle *>(arg);

	// Block the internal delivery signal in this thread so timer expirations stay pending until we
	// sigtimedwait them (never delivered as an unmasked default action).
	sigset_t blocked;
	sigemptyset(&blocked);
	sigaddset(&blocked, roxyTimerSignal);
	pthread_sigmask(SIG_BLOCK, &blocked, nullptr);

	// Publish this thread's kernel tid; the creating thread waits on `armed` before it creates the
	// kernel timer targeted at this thread, so no delivery can precede the blocking above.
	h->helperTid = static_cast<int32_t>(mlibc::sysdep<GetTid>());
	__atomic_store_n(&h->armed, 1, __ATOMIC_RELEASE);
	mlibc::sysdep<FutexWake>(&h->armed, true);

	sigset_t oneset;
	sigemptyset(&oneset);
	sigaddset(&oneset, roxyTimerSignal);

	while (!__atomic_load_n(&h->quit, __ATOMIC_RELAXED)) {
		int signo = 0;
		int error = mlibc::sysdep<Sigtimedwait>(&oneset, nullptr, nullptr, &signo);
		// With no timeout this never times out; a spurious EINTR just loops.
		if (error && error != EINTR)
			continue;
		if (signo == roxyTimerSignal && !__atomic_load_n(&h->quit, __ATOMIC_RELAXED))
			h->notify.sigev_notify_function(h->notify.sigev_value);
	}

	return nullptr;
}

// Stops and reaps the helper thread of a SIGEV_THREAD timer.
void stopHelper(roxyTimerHandle *h) {
	__atomic_store_n(&h->quit, 1, __ATOMIC_RELAXED);
	// Wake the sigtimedwait with the internal signal so the helper observes `quit` promptly.
	mlibc::sysdep<Tgkill>(static_cast<int>(mlibc::sysdep<GetPid>()), h->helperTid, roxyTimerSignal);
	pthread_join(h->helper, nullptr);
}

// The single POSIX timer backing `ITIMER_REAL`. POSIX keeps exactly one per process, so we create
// it lazily once and reuse it across setitimer/getitimer calls (never delete: arming with an
// it_value of zero disarms without dropping the handle). Delivering SIGALRM matches alarm(3).
timer_t realIntervalTimer = nullptr;
pthread_mutex_t intervalTimerLock = PTHREAD_MUTEX_INITIALIZER;

void itimervalToItimerspec(const struct itimerval *src, struct itimerspec *dst) {
	dst->it_value.tv_sec = src->it_value.tv_sec;
	dst->it_value.tv_nsec = src->it_value.tv_usec * 1000;
	dst->it_interval.tv_sec = src->it_interval.tv_sec;
	dst->it_interval.tv_nsec = src->it_interval.tv_usec * 1000;
}

void itimerspecToItimerval(const struct itimerspec *src, struct itimerval *dst) {
	dst->it_value.tv_sec = src->it_value.tv_sec;
	dst->it_value.tv_usec = src->it_value.tv_nsec / 1000;
	dst->it_interval.tv_sec = src->it_interval.tv_sec;
	dst->it_interval.tv_usec = src->it_interval.tv_nsec / 1000;
}

} // namespace

int Sysdeps<ClockGet>::operator()(int clock, time_t *secs, long *nanos) {
	roxy_clock_result result;
	auto error =
	    syscall_error(roxy_syscall2(ROXY_SYS_CLOCK_GET, clock, reinterpret_cast<long>(&result)));
	if (error)
		return error;

	*secs = result.seconds;
	*nanos = result.nanoseconds;
	return 0;
}

// Reports the interval in which a clock advances, in the same record `clock_get` fills. The kernel
// advances its clocks one periodic-timer tick at a time, and returns EINVAL for an identifier it
// does not provide, which lets a caller fall back to another clock.
int Sysdeps<ClockGetres>::operator()(int clock, time_t *secs, long *nanos) {
	roxy_clock_result result;
	auto error = syscall_error(
	    roxy_syscall2(ROXY_SYS_CLOCK_GETRES, clock, reinterpret_cast<long>(&result))
	);
	if (error)
		return error;

	*secs = result.seconds;
	*nanos = result.nanoseconds;
	return 0;
}

int Sysdeps<Sleep>::operator()(time_t *secs, long *nanos) {
	struct timespec request = {
	    .tv_sec = *secs,
	    .tv_nsec = *nanos,
	};

	auto error = syscall_error(
	    roxy_syscall1(ROXY_SYS_SLEEP, reinterpret_cast<long>(&request))
	);
	if(error)
		return error;

	return 0;
}

int Sysdeps<TimerCreate>::operator()(clockid_t clk, struct sigevent *evp, timer_t *res) {
	struct sigevent local{};
	if (evp) {
		local = *evp;
	} else {
		local.sigev_notify = SIGEV_SIGNAL;
		local.sigev_signo = SIGALRM;
	}

	auto *h = static_cast<roxyTimerHandle *>(calloc(1, sizeof(roxyTimerHandle)));
	if (!h)
		return ENOMEM;
	h->notify = local;
	h->notifyMode = local.sigev_notify;

	if (local.sigev_notify == SIGEV_THREAD) {
		if (int e = pthread_create(
		        &h->helper, h->notify.sigev_notify_attributes, roxyTimerHelper, h)) {
			free(h);
			return e;
		}

		// Wait for the helper to have blocked the internal signal and published its tid.
		while (__atomic_load_n(&h->armed, __ATOMIC_ACQUIRE) == 0)
			mlibc::sysdep<FutexWait>(&h->armed, 0, nullptr);

		struct sigevent kevent{};
		kevent.sigev_notify = SIGEV_THREAD_ID;
		kevent.sigev_signo = roxyTimerSignal;
		kevent.sigev_value = h->notify.sigev_value;
		kevent.sigev_notify_thread_id = h->helperTid;

		int32_t kernelId = 0;
		auto result = roxy_syscall3(
		    ROXY_SYS_TIMER_CREATE, clk, reinterpret_cast<long>(&kevent),
		    reinterpret_cast<long>(&kernelId)
		);
		if (result.error) {
			stopHelper(h);
			free(h);
			return static_cast<int>(result.error);
		}
		h->kernel_id = kernelId;
		*res = reinterpret_cast<timer_t>(h);
		return 0;
	}

	// SIGEV_NONE and SIGEV_SIGNAL go straight to the kernel timer with the caller's sigevent
	// (null means the SIGALRM default).
	int32_t kernelId = 0;
	auto result = roxy_syscall3(
	    ROXY_SYS_TIMER_CREATE, clk, reinterpret_cast<long>(evp),
	    reinterpret_cast<long>(&kernelId)
	);
	if (result.error) {
		free(h);
		return static_cast<int>(result.error);
	}
	h->kernel_id = kernelId;
	*res = reinterpret_cast<timer_t>(h);
	return 0;
}

int Sysdeps<TimerSettime>::operator()(
    timer_t t, int flags, const struct itimerspec *val, struct itimerspec *old) {
	auto *h = reinterpret_cast<roxyTimerHandle *>(t);
	return syscall_error(roxy_syscall4(
	    ROXY_SYS_TIMER_SETTIME, h->kernel_id, flags, reinterpret_cast<long>(val),
	    reinterpret_cast<long>(old)
	));
}

int Sysdeps<TimerGettime>::operator()(timer_t t, struct itimerspec *val) {
	auto *h = reinterpret_cast<roxyTimerHandle *>(t);
	return syscall_error(roxy_syscall2(
	    ROXY_SYS_TIMER_GETTIME, h->kernel_id, reinterpret_cast<long>(val)
	));
}

int Sysdeps<TimerGetoverrun>::operator()(timer_t t, int *out) {
	auto *h = reinterpret_cast<roxyTimerHandle *>(t);
	return syscall_error(roxy_syscall2(
	    ROXY_SYS_TIMER_GETOVERRUN, h->kernel_id, reinterpret_cast<long>(out)
	));
}

int Sysdeps<TimerDelete>::operator()(timer_t t) {
	auto *h = reinterpret_cast<roxyTimerHandle *>(t);
	if (h->notifyMode == SIGEV_THREAD)
		stopHelper(h);

	int error = syscall_error(roxy_syscall1(ROXY_SYS_TIMER_DELETE, h->kernel_id));
	free(h);
	return error;
}

int Sysdeps<GetItimer>::operator()(int which, struct itimerval *curr_value) {
	if (which != ITIMER_REAL)
		return EINVAL; // ITIMER_VIRTUAL/PROF need a CPU-time clock Roxy does not (yet) expose.

	pthread_mutex_lock(&intervalTimerLock);
	int error = 0;
	if (realIntervalTimer) {
		struct itimerspec cur = {};
		error = mlibc::sysdep<TimerGettime>(realIntervalTimer, &cur);
		if (!error)
			itimerspecToItimerval(&cur, curr_value);
	} else {
		// A timer that was never armed reports zero remaining time.
		curr_value->it_value.tv_sec = 0;
		curr_value->it_value.tv_usec = 0;
		curr_value->it_interval.tv_sec = 0;
		curr_value->it_interval.tv_usec = 0;
	}
	pthread_mutex_unlock(&intervalTimerLock);
	return error;
}

int Sysdeps<SetItimer>::operator()(
    int which, const struct itimerval *new_value, struct itimerval *old_value) {
	if (which != ITIMER_REAL)
		return EINVAL; // ITIMER_VIRTUAL/PROF need a CPU-time clock Roxy does not (yet) expose.

	pthread_mutex_lock(&intervalTimerLock);
	int error = 0;

	if (!realIntervalTimer) {
		struct sigevent ev = {};
		ev.sigev_notify = SIGEV_SIGNAL;
		ev.sigev_signo = SIGALRM;
		error = mlibc::sysdep<TimerCreate>(CLOCK_REALTIME, &ev, &realIntervalTimer);
	}

	if (!error && old_value) {
		struct itimerspec cur = {};
		error = mlibc::sysdep<TimerGettime>(realIntervalTimer, &cur);
		if (!error)
			itimerspecToItimerval(&cur, old_value);
	}

	if (!error && new_value) {
		struct itimerspec nxt = {};
		itimervalToItimerspec(new_value, &nxt);
		error = mlibc::sysdep<TimerSettime>(realIntervalTimer, 0, &nxt, nullptr);
	}

	pthread_mutex_unlock(&intervalTimerLock);
	return error;
}

} // namespace mlibc