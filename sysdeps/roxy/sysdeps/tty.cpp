#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include "errors.hpp"

namespace mlibc {

int Sysdeps<Isatty>::operator()(int fd) {
	return syscall_error(roxy_syscall1(ROXY_SYS_ISATTY, fd));
}

int Sysdeps<Ttyname>::operator()(int fd, char *buf, size_t size) {
	return syscall_error(
	    roxy_syscall3(ROXY_SYS_TTYNAME, fd, reinterpret_cast<long>(buf), size)
	);
}

} // namespace mlibc
