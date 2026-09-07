#include <errno.h>
#include <bits/ensure.h>
#include <mlibc/all-sysdeps.hpp>
#include <mlibc/tcb.hpp>
#include <mlibc/threads.hpp>
#include <roxy/syscall.h>

#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

extern "C" void __mlibc_enter_thread(void *entry, void *user_arg, Tcb *tcb) {
	// Point the thread's FS base at its own TCB so get_current_tcb() works for this thread.
	if (mlibc::sysdep<TcbSet>(tcb))
		__ensure(!"failed to set tcb for the new thread");

	// Wait until the creating thread publishes this thread's TID in the shared TCB.
	while (__atomic_load_n(&tcb->tid, __ATOMIC_RELAXED) == 0)
		mlibc::sysdep<FutexWait>(&tcb->tid, 0, nullptr);

	// Enable cancellation once the TCB is fully set up.
	__atomic_fetch_or(&tcb->cancelBits, tcbCancelEnableBit, __ATOMIC_RELAXED);

	tcb->invokeThreadFunc(entry, user_arg);
	mlibc::thread_exit(tcb->returnValue);
}

// The child thread's entry, defined in arch/x86_64/thread_entry.S. It pops the
// entry/user-arg/tcb triple placed on the stack by PrepareStack and calls enter_thread.
extern "C" void __mlibc_start_thread();

namespace mlibc {

static constexpr size_t defaultStackSize = 0x200000;

int Sysdeps<PrepareStack>::operator()(
	void **stack,
	void *entry,
	void *user_arg,
	void *tcb,
	size_t *stack_size,
	size_t *guard_size,
	void **stack_base
) {
	if (!*stack_size)
		*stack_size = defaultStackSize;
	*guard_size = 0;

	if (*stack) {
		*stack_base = *stack;
	} else {
		*stack_base = mmap(
			nullptr,
			*stack_size,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1,
			0
		);
		if (*stack_base == MAP_FAILED)
			return errno;
	}

	// The child starts by popping tcb, user_arg, and entry off the top of its stack, so the
	// triple is pushed in reverse order with the entry at the lowest (first-popped) address.
	uintptr_t *sp = reinterpret_cast<uintptr_t *>(
		reinterpret_cast<uintptr_t>(*stack_base) + *stack_size);

	*--sp = reinterpret_cast<uintptr_t>(tcb);
	*--sp = reinterpret_cast<uintptr_t>(user_arg);
	*--sp = reinterpret_cast<uintptr_t>(entry);
	*stack = reinterpret_cast<void *>(sp);
	return 0;
}

int Sysdeps<Clone>::operator()(void *tcb, pid_t *pid_out, void *stack) {
	(void)tcb;
	auto result = roxy_syscall2(
		ROXY_SYS_THREAD_CREATE,
		static_cast<roxy_syscall_word_t>(
			reinterpret_cast<uintptr_t>(__mlibc_start_thread)
		),
		reinterpret_cast<roxy_syscall_word_t>(stack)
	);
	if (result < 0)
		return static_cast<int>(-result);

	*pid_out = static_cast<pid_t>(result);
	return 0;
}

void Sysdeps<ThreadExit>::operator()() {
	roxy_syscall0(ROXY_SYS_THREAD_EXIT);
	__builtin_trap();
}

pid_t Sysdeps<GetTid>::operator()() {
	// gettid() always succeeds.
	return static_cast<pid_t>(roxy_syscall0(ROXY_SYS_GET_TID));
}

} // namespace mlibc