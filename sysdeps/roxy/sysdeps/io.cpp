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

long posix_open_access_to_roxy(int flags) {
	switch(flags & O_ACCMODE) {
		case O_RDONLY: return ROXY_OPEN_ACCESS_READ_ONLY;
		case O_WRONLY: return ROXY_OPEN_ACCESS_WRITE_ONLY;
		case O_RDWR: return ROXY_OPEN_ACCESS_READ_WRITE;
		default: __builtin_unreachable();
	}
}

bool posix_open_flags_to_roxy(int flags, long *roxy_flags) {
	int supported = O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC;
#ifdef O_LARGEFILE
	supported |= O_LARGEFILE;
#endif
	if(flags & ~supported)
		return false;

	long translated = 0;
	if(flags & O_CREAT) translated |= ROXY_OPEN_CREATE;
	if(flags & O_EXCL) translated |= ROXY_OPEN_EXCLUSIVE;
	if(flags & O_TRUNC) translated |= ROXY_OPEN_TRUNCATE;
	if(flags & O_APPEND) translated |= ROXY_OPEN_APPEND;
	if(flags & O_NONBLOCK) translated |= ROXY_OPEN_NONBLOCK;
	if(flags & O_NOFOLLOW) translated |= ROXY_OPEN_NOFOLLOW;
#ifdef O_LARGEFILE
	if(flags & O_LARGEFILE) translated |= ROXY_OPEN_LARGE_FILE;
#endif
	if(flags & O_CLOEXEC) translated |= ROXY_OPEN_CLOEXEC;
	*roxy_flags = translated;
	return true;
}

} // namespace

int Sysdeps<Open>::operator()(const char *path, int flags, mode_t mode, int *fd) {
	long roxy_flags = 0;
	if(!posix_open_flags_to_roxy(flags, &roxy_flags))
		return EINVAL;

	roxy_open_request request = {};
	request.access = posix_open_access_to_roxy(flags);
	request.flags = roxy_flags;
	request.mode = mode;

	auto result = roxy_syscall2(
	    ROXY_SYS_OPEN,
	    reinterpret_cast<long>(path),
	    reinterpret_cast<long>(&request)
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
