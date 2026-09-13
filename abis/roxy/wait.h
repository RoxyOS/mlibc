#ifndef _ABIBITS_WAIT_H
#define _ABIBITS_WAIT_H

#include <mlibc-config.h>

/*
 * Roxy's `wait` option bits.
 *
 * Bits are numbered from `ROXY_WAIT_OPTIONS_BASE`, which sits above Linux's *whole* option range
 * (its highest, `WNOWAIT`, is bit 24), so no Linux bit can alias one of ours: a word carrying any
 * bit below the base is another personality's numbering and the kernel reports it as foreign.
 *
 * Each option owns a bit instead of sharing one, so a caller's `&` test for one option cannot
 * answer true because another was requested. `WEXITED` and `WNOWAIT` belong to `waitid`, which
 * Roxy does not implement; they are defined so that ported code names them, and the kernel
 * reports them when a caller passes them.
 *
 * The kernel side is `kernel/syscall/src/syscalls/waitpid.rs`.
 */
#define ROXY_WAIT_OPTIONS_BASE (1 << 25)

#define WNOHANG ROXY_WAIT_OPTIONS_BASE
#define WUNTRACED (ROXY_WAIT_OPTIONS_BASE << 1)
#define WSTOPPED WUNTRACED
#define WCONTINUED (ROXY_WAIT_OPTIONS_BASE << 2)
#define WEXITED (ROXY_WAIT_OPTIONS_BASE << 3)
#define WNOWAIT (ROXY_WAIT_OPTIONS_BASE << 4)

#if __MLIBC_LINUX_OPTION

#define __WALL 0x40000000
#define __WCLONE 0x80000000

#endif /* __MLIBC_LINUX_OPTION */

#define __WCOREFLAG 0x80

#define WEXITSTATUS(x) (((x) & 0xff00) >> 8)
#define WTERMSIG(x) ((x) & 0x7f)
#define WSTOPSIG(x) WEXITSTATUS(x)
#define WIFEXITED(x) (WTERMSIG(x) == 0)
#define WIFSIGNALED(x) (((signed char) (((x) & 0x7f) + 1) >> 1) > 0)
#define WIFSTOPPED(x) (((x) & 0xff) == 0x7f)
#define WIFCONTINUED(x) ((x) == 0xffff)

#if defined(_DEFAULT_SOURCE)
#define WCOREFLAG __WCOREFLAG
#endif

#if defined(_DEFAULT_SOURCE) || __MLIBC_POSIX2024
#define WCOREDUMP(x) ((x) & __WCOREFLAG)
#endif /* defined(_DEFAULT_SOURCE) || __MLIBC_POSIX2024 */

/* glibc extension, but also useful for kernels */
#if defined(_DEFAULT_SOURCE)
#define W_EXITCODE(ret, sig) (((ret) << 8) | (sig))
#endif /* defined(_DEFAULT_SOURCE) */

#endif /*_ABIBITS_WAIT_H */
