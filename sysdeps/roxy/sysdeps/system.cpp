#include <errno.h>
#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include <string.h>

#include "errors.hpp"

namespace mlibc {

int Sysdeps<Uname>::operator()(struct utsname *output) {
	return syscall_error(roxy_syscall1(ROXY_SYS_UNAME, reinterpret_cast<long>(output)));
}

int Sysdeps<GetHostname>::operator()(char *buffer, size_t bufsize) {
	const char *hostname = "roxybestgirl";
	size_t length = strlen(hostname);

	if(length >= bufsize)
		return ENAMETOOLONG;

	memcpy(buffer, hostname, length + 1);
	return 0;
}

} // namespace mlibc
