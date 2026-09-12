#ifndef ROXY_SYSDEPS_ERRORS_HPP
#define ROXY_SYSDEPS_ERRORS_HPP

#include <bits/ssize_t.h>

// The kernel reports a failed syscall as a negative errno in its result word. mlibc's sysdeps
// instead return an errno and report a successful result through an out parameter, so every
// syscall-backed implementation goes through one of these two conversions.
namespace mlibc {

inline int syscall_error(long result) {
	return result < 0 ? static_cast<int>(-result) : 0;
}

inline int syscall_result(long result, ssize_t *transferred) {
	if(result < 0)
		return static_cast<int>(-result);

	*transferred = result;
	return 0;
}

} // namespace mlibc

#endif
