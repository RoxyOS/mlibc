#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include "errors.hpp"

namespace mlibc {

int Sysdeps<Kill>::operator()(pid_t pid, int signal) {
	return syscall_error(roxy_syscall2(ROXY_SYS_SEND_SIGNAL, pid, signal));
}

int Sysdeps<Sigprocmask>::operator()(
	int how,
	const sigset_t *__restrict set,
	sigset_t *__restrict retrieve
) {
	return syscall_error(roxy_syscall3(
	    ROXY_SYS_SIGPROCMASK,
	    how,
	    reinterpret_cast<long>(set),
	    reinterpret_cast<long>(retrieve)
	));
}

int Sysdeps<Sigaction>::operator()(
	int signal,
	const struct sigaction *__restrict action,
	struct sigaction *__restrict old_action
) {
	return syscall_error(roxy_syscall3(
	    ROXY_SYS_SIGACTION, signal, reinterpret_cast<long>(action), reinterpret_cast<long>(old_action)
	));
}

int Sysdeps<ThreadSigmask>::operator()(
	int how,
	const sigset_t *__restrict set,
	sigset_t *__restrict retrieve
) {
	// Roxy's SIGPROCMASK is scoped to the calling thread, which is exactly what pthread_sigmask
	// needs; the `ThreadSigmask` sysdep is provided so `pthread_sigmask` is selected over the
	// process-wide `Sigprocmask` fallback.
	return syscall_error(roxy_syscall3(
	    ROXY_SYS_SIGPROCMASK, how, reinterpret_cast<long>(set), reinterpret_cast<long>(retrieve)
	));
}

int Sysdeps<Sigtimedwait>::operator()(
	const sigset_t *__restrict set,
	siginfo_t *__restrict info,
	const struct timespec *__restrict timeout,
	int *out_signal
) {
	return syscall_error(roxy_syscall4(
	    ROXY_SYS_SIGTIMEDWAIT, reinterpret_cast<long>(set), reinterpret_cast<long>(info),
	    reinterpret_cast<long>(timeout), reinterpret_cast<long>(out_signal)
	));
}

int Sysdeps<Tgkill>::operator()(int tgid, int tid, int sig) {
	return syscall_error(roxy_syscall3(ROXY_SYS_TGKILL, tgid, tid, sig));
}

} // namespace mlibc
