#include <errno.h>
#include <stdarg.h>
#include <sys/ioctl.h>

#include <mlibc/all-sysdeps.hpp>

/*
 * The public ioctl() wrapper. Roxy's sysdeps own this instead of taking it from mlibc's
 * glibc option, like every other Unix libc provides ioctl itself.
 *
 * SAFETY-ish note on the variadic argument: ioctl takes at most one argument beyond the
 * request, and every caller in the request number's contract passes either a pointer to a
 * record or an integer widened to a pointer (TCFLSH's queue selector). Reading one
 * `void *` therefore covers all of them; requests that take no argument ignore it.
 */
int ioctl(int fd, unsigned long request, ...) {
	va_list args;
	va_start(args, request);
	void *argument = va_arg(args, void *);

	int result = 0;
	if (int error = mlibc::sysdep_or_enosys<Ioctl>(fd, request, argument, &result)) {
		va_end(args);
		errno = error;
		return -1;
	}

	va_end(args);
	return result;
}
