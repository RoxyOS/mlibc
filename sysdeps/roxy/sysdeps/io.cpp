#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include <stdarg.h>

#include "errors.hpp"

namespace mlibc {

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
	// dup(fd) = fcntl(fd, F_DUPFD, 0): returns the lowest available fd >= 0.
	auto raw = roxy_syscall3(ROXY_SYS_FCNTL, fd, 0 /* F_DUPFD */, 0);
	if(int error = syscall_error(raw); error)
		return error;
	*newfd = static_cast<int>(raw.value);
	return 0;
}

int Sysdeps<Dup2>::operator()(int oldfd, int flags, int newfd) {
	// The kernel ABI takes (oldfd, newfd, flags); the mlibc tag passes (fd, flags, newfd).
	auto result = roxy_syscall3(ROXY_SYS_DUP2, oldfd, newfd, flags);
	return syscall_error(result);
}

int Sysdeps<Fcntl>::operator()(int fd, int command, va_list args, int *result) {
	auto argument = va_arg(args, unsigned long);
	auto raw = roxy_syscall3(ROXY_SYS_FCNTL, fd, command, argument);
	if(int error = syscall_error(raw); error)
		return error;

	*result = static_cast<int>(raw.value);
	return 0;
}

} // namespace mlibc
