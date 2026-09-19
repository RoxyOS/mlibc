#include <errno.h>
#include <fcntl.h>
#include <mlibc/all-sysdeps.hpp>
#include <mlibc/debug.hpp>
#include <roxy/syscall.h>

#include <stdarg.h>

#include "errors.hpp"

namespace mlibc {

namespace {

long posix_descriptor_flags_to_roxy(int flags) {
	return (flags & FD_CLOEXEC) ? ROXY_DESCRIPTOR_CLOSE_ON_EXEC : 0;
}

int roxy_descriptor_flags_to_posix(long flags) {
	return (flags & ROXY_DESCRIPTOR_CLOSE_ON_EXEC) ? FD_CLOEXEC : 0;
}

long posix_dup_options_to_roxy(int flags, bool minimum_argument) {
	long options = minimum_argument ? ROXY_DUP_MINIMUM_ARGUMENT : 0;
	if(flags & O_CLOEXEC)
		options |= ROXY_DUP_CLOSE_ON_EXEC;
	return options;
}

} // namespace

int Sysdeps<Open>::operator()(const char *path, int flags, mode_t mode, int *fd) {
	auto result = roxy_syscall3(
	    ROXY_SYS_OPEN,
	    reinterpret_cast<long>(path),
	    flags,
	    mode
	);
	if(result.error)
		return static_cast<int>(result.error);

	*fd = static_cast<int>(result.value);
	return 0;
}

int Sysdeps<Read>::operator()(int fd, void *buffer, size_t count, ssize_t *bytes_read) {
	return syscall_result(
	    roxy_syscall3(ROXY_SYS_READ, fd, reinterpret_cast<long>(buffer), count), bytes_read
	);
}

int Sysdeps<Write>::operator()(int fd, const void *buffer, size_t count, ssize_t *bytes_written) {
	return syscall_result(
	    roxy_syscall3(ROXY_SYS_WRITE, fd, reinterpret_cast<long>(buffer), count), bytes_written
	);
}

int Sysdeps<Writev>::operator()(int fd, const struct iovec *iovs, int iovc, ssize_t *bytes_written) {
	return syscall_result(
	    roxy_syscall3(ROXY_SYS_WRITEV, fd, reinterpret_cast<long>(iovs), iovc), bytes_written
	);
}

int Sysdeps<Close>::operator()(int fd) {
	return syscall_error(roxy_syscall1(ROXY_SYS_CLOSE, fd));
}

int Sysdeps<Pipe>::operator()(int *fds, int flags) {
	auto result = roxy_syscall2(ROXY_SYS_PIPE, reinterpret_cast<long>(fds), flags);
	return syscall_error(result);
}

int Sysdeps<Dup>::operator()(int fd, int flags, int *newfd) {
	// `dup` takes the options word and, when the caller named one, the minimum descriptor.
	// Without the minimum option the kernel searches from zero, so the third argument is
	// passed only to carry it.
	long options = posix_dup_options_to_roxy(flags, false);

	auto raw = roxy_syscall3(ROXY_SYS_DUP, fd, options, 0);
	if(int error = syscall_error(raw); error)
		return error;
	*newfd = static_cast<int>(raw.value);
	return 0;
}

int Sysdeps<Dup2>::operator()(int oldfd, int flags, int newfd) {
	// The kernel ABI takes (oldfd, newfd, descriptor flags); the mlibc tag passes
	// (fd, flags, newfd). The flags are POSIX's close-on-exec, which is the one descriptor flag
	// the kernel has.
	long descriptor_flags = posix_descriptor_flags_to_roxy(flags);

	auto result = roxy_syscall3(ROXY_SYS_DUP_ONTO, oldfd, newfd, descriptor_flags);
	return syscall_error(result);
}

int Sysdeps<Fcntl>::operator()(int fd, int command, va_list args, int *result) {
	switch(command) {
		case F_DUPFD:
		case F_DUPFD_CLOEXEC: {
			auto argument = va_arg(args, int);
			long options = posix_dup_options_to_roxy(
			    command == F_DUPFD_CLOEXEC ? O_CLOEXEC : 0,
			    true
			);

			auto raw = roxy_syscall3(ROXY_SYS_DUP, fd, options, static_cast<long>(argument));
			if(int error = syscall_error(raw); error)
				return error;
			*result = static_cast<int>(raw.value);
			return 0;
		}

		case F_GETFD: {
			auto raw = roxy_syscall1(ROXY_SYS_GET_DESCRIPTOR_FLAGS, fd);
			if(int error = syscall_error(raw); error)
				return error;

			// POSIX spells close-on-exec as 1; the kernel reports its own word.
			*result = roxy_descriptor_flags_to_posix(raw.value);
			return 0;
		}

		case F_SETFD: {
			auto argument = va_arg(args, int);
			long descriptor_flags = posix_descriptor_flags_to_roxy(argument);

			return syscall_error(
			    roxy_syscall2(ROXY_SYS_SET_DESCRIPTOR_FLAGS, fd, descriptor_flags)
			);
		}

		case F_GETFL: {
			auto raw = roxy_syscall1(ROXY_SYS_GET_STATUS_FLAGS, fd);
			if(int error = syscall_error(raw); error)
				return error;

			// The status word is the `O_*` word minus the flags that only describe creation, so a
			// caller can hand the result straight back to `F_SETFL`.
			*result = static_cast<int>(raw.value) & ROXY_STATUS_FLAGS_MASK;
			return 0;
		}

		case F_SETFL: {
			auto argument = va_arg(args, int);
			// The access mode is preserved by the kernel, which is what F_SETFL requires; every
			// other bit is either applied or reported by the kernel's own parser.
			return syscall_error(
			    roxy_syscall2(ROXY_SYS_SET_STATUS_FLAGS, fd, static_cast<long>(argument))
			);
		}
	}

	// Every other command is one this library does not serve. The kernel serves none of them
	// either, so the diagnostic is emitted here rather than at the syscall boundary.
	mlibc::infoLogger() << "fcntl: unsupported command " << command << frg::endlog;
	return ENOTSUP;
}

} // namespace mlibc
