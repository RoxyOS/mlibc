#include <stddef.h>
#include <mlibc/all-sysdeps.hpp>
#include <sys/ioctl.h>

#include "errors.hpp"

namespace mlibc {

namespace {

struct RoxyTtyNameRequest {
	char *buffer;
	size_t capacity;
	size_t required;
};

static_assert(sizeof(RoxyTtyNameRequest) == 24);
static_assert(alignof(RoxyTtyNameRequest) == 8);
static_assert(offsetof(RoxyTtyNameRequest, buffer) == 0);
static_assert(offsetof(RoxyTtyNameRequest, capacity) == 8);
static_assert(offsetof(RoxyTtyNameRequest, required) == 16);

} // namespace

int Sysdeps<Isatty>::operator()(int fd) {
	return syscall_error(roxy_syscall1(ROXY_SYS_ISATTY, fd));
}

int Sysdeps<Ttyname>::operator()(int fd, char *buf, size_t size) {
	RoxyTtyNameRequest request{buf, size, 0};
	return sysdep<Ioctl>(fd, TIOCGNAME, &request, nullptr);
}

} // namespace mlibc
