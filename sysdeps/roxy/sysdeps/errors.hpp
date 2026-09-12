#ifndef ROXY_SYSDEPS_ERRORS_HPP
#define ROXY_SYSDEPS_ERRORS_HPP

#include <roxy/syscall.h>

#include <bits/ssize_t.h>

// The kernel reports a syscall's value and its error code in separate registers, and `error` is 0
// when the call succeeded, so it must be checked before the value is used. mlibc's sysdeps instead
// return an errno and report a successful result through an out parameter, so every syscall-backed
// implementation goes through one of these two conversions.
namespace mlibc {

inline int syscall_error(roxy_syscall_result result) {
	return static_cast<int>(result.error);
}

inline int syscall_result(roxy_syscall_result result, ssize_t *transferred) {
	if(result.error)
		return static_cast<int>(result.error);

	*transferred = result.value;
	return 0;
}

} // namespace mlibc

#endif
