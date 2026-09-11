#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

/*
 * Roxy's ioctl entry point. `abi-bits/ioctls.h` owns the request numbers; this header
 * declares the call itself, which every Unix-family libc provides: ioctl is neither ISO C
 * nor POSIX, but it has been part of every Unix since Version 7, and code as ordinary as
 * busybox's `libbb.h` includes this header unconditionally.
 *
 * Only the requests Roxy implements are defined, in `abi-bits/ioctls.h`. A request the
 * kernel does not implement has no definition here: code that asks for one should either
 * skip it under a capability test or be fixed, rather than compile against a number whose
 * only possible answer is ENOTTY.
 */

#include <mlibc-config.h>
#include <abi-bits/ioctls.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MLIBC_ABI_ONLY

int ioctl(int __fd, unsigned long __request, ...);

#endif /* !__MLIBC_ABI_ONLY */

#ifdef __cplusplus
}
#endif

#endif /* _SYS_IOCTL_H */
