#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include <stdint.h>

#include "errors.hpp"

namespace mlibc {

int Sysdeps<FutexWait>::operator()(int *pointer, int expected, const timespec *timeout) {
	return syscall_error(roxy_syscall3(
	    ROXY_SYS_FUTEX_WAIT,
	    reinterpret_cast<long>(pointer),
	    expected,
	    reinterpret_cast<long>(timeout)
	));
}

int Sysdeps<FutexWake>::operator()(int *pointer, bool all) {
	return syscall_error(roxy_syscall2(
	    ROXY_SYS_FUTEX_WAKE,
	    reinterpret_cast<long>(pointer),
	    all ? UINT32_MAX : 1
	));
}

pid_t Sysdeps<FutexTid>::operator()() {
	// gettid() always succeeds.
	return static_cast<pid_t>(roxy_syscall0(ROXY_SYS_GET_TID));
}

} // namespace mlibc
