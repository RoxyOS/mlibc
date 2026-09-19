#ifndef _ABIBITS_FCNTL_H
#define _ABIBITS_FCNTL_H

#include <mlibc-config.h>
#include <abi-bits/pid_t.h>

#define O_PATH 010000000

#define O_ACCMODE (03 | O_PATH)
#define O_RDONLY   00
#define O_WRONLY   01
#define O_RDWR     02

#define O_CREAT         0100
#define O_EXCL          0200
#define O_NOCTTY        0400
#define O_TRUNC        01000
#define O_APPEND       02000
#define O_NONBLOCK     04000
#define O_DSYNC       010000
#define O_ASYNC       020000
#define O_CLOEXEC   02000000
#define O_SYNC      04010000
#define O_RSYNC     04010000
#define O_NOATIME   01000000

#if defined(__x86_64__) || defined(__i386__) || defined(__riscv) || defined(__loongarch64)
#define O_DIRECT      040000
#define O_LARGEFILE  0100000
#define O_DIRECTORY  0200000
#define O_NOFOLLOW   0400000
#elif defined(__aarch64__) || defined(__m68k__)
#define O_DIRECTORY   040000
#define O_NOFOLLOW   0100000
#define O_DIRECT     0200000
#define O_LARGEFILE  0400000
#else
#warning "Missing <fcntl.h> support for this architecture!"
#endif

#define O_TMPFILE (020000000 | O_DIRECTORY)

#define O_EXEC O_PATH
#define O_SEARCH O_PATH
#define O_TTY_INIT 0

/*
 * `F_GETFL` reports the file access mode and the file status flags, but not flags that only
 * describe how a path was opened or resolved. The kernel's status word uses the same values as
 * the Roxy open/status implementation, so the sysdep masks it to the application-visible access
 * and status flags before returning it to POSIX callers.
 */
#define ROXY_STATUS_FLAGS_MASK (O_ACCMODE | O_APPEND | O_NONBLOCK | O_DSYNC | O_ASYNC | O_DIRECT | O_LARGEFILE | O_NOATIME)

/*
 * The Roxy `fcntl` command space.
 *
 * These numbers belong to the Roxy personality even though they are consumed by the libc
 * `fcntl` wrapper rather than sent to the kernel. The wrapper translates each supported command
 * into the operation-specific syscall in `kernel/syscall/src/syscalls/fd`, and reports the common
 * `F_UNSUPPORTED` marker for every command the personality does not serve. Keeping the command
 * word Roxy-owned prevents a caller compiled against another personality from being mistaken for
 * a Roxy command before that translation occurs.
 */
#define ROXY_F_CMD_BASE 0x1000u
#define F_UNSUPPORTED 0x100u

/* Commands this library serves; a value is the Roxy base plus the command's index. */
#define F_DUPFD (ROXY_F_CMD_BASE + 0)
#define F_GETFD (ROXY_F_CMD_BASE + 1)
#define F_SETFD (ROXY_F_CMD_BASE + 2)
#define F_GETFL (ROXY_F_CMD_BASE + 3)
#define F_SETFL (ROXY_F_CMD_BASE + 4)
#define F_DUPFD_CLOEXEC (ROXY_F_CMD_BASE + 5)

/* Commands this library does not serve. The `64` spellings are kept because ported sources name
   them explicitly. */
#define F_GETLK64 F_UNSUPPORTED
#define F_SETLK64 F_UNSUPPORTED
#define F_SETLKW64 F_UNSUPPORTED
#define F_GETLK F_UNSUPPORTED
#define F_SETLK F_UNSUPPORTED
#define F_SETLKW F_UNSUPPORTED
#define F_SETOWN F_UNSUPPORTED
#define F_GETOWN F_UNSUPPORTED
#define F_SETSIG F_UNSUPPORTED
#define F_GETSIG F_UNSUPPORTED
#define F_SETOWN_EX F_UNSUPPORTED
#define F_GETOWN_EX F_UNSUPPORTED
#define F_GETOWNER_UIDS F_UNSUPPORTED
#define F_SETLEASE F_UNSUPPORTED
#define F_GETLEASE F_UNSUPPORTED
#define F_NOTIFY F_UNSUPPORTED
#define F_DUPFD_QUERY F_UNSUPPORTED
#define F_SETPIPE_SZ F_UNSUPPORTED
#define F_GETPIPE_SZ F_UNSUPPORTED
#define F_ADD_SEALS F_UNSUPPORTED
#define F_GET_SEALS F_UNSUPPORTED
#define F_OFD_GETLK F_UNSUPPORTED
#define F_OFD_SETLK F_UNSUPPORTED
#define F_OFD_SETLKW F_UNSUPPORTED

/* Arguments of unsupported commands; the kernel never interprets them. */
#define F_SEAL_SEAL 0x0001
#define F_SEAL_SHRINK 0x0002
#define F_SEAL_GROW 0x0004
#define F_SEAL_WRITE 0x0008

/* `struct flock.l_type` values; record locking itself is unsupported. */
#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

/* The argument bit of the supported `F_GETFD`/`F_SETFD` commands. */
#define FD_CLOEXEC 1

/*
 * Roxy's `dirfd` selector and `AT_*` flags.
 *
 * The two are separate arguments with separate shapes. `dirfd` is a descriptor plus one magic
 * selector: a descriptor is never negative, so the working directory is spelled as a negative
 * value no descriptor can hold. A negative value this header does not name — Linux's own
 * `AT_FDCWD` is -100 — is another personality's numbering, and the kernel reports such a caller as
 * foreign instead of reading it as a request of its own.
 *
 * `AT_*` is a flag word, so every flag is one bit from a base above Linux's whole range for the
 * word, whose top is `AT_RECURSIVE` at bit 15: a lower base would let one of Linux's flags land on
 * one of ours, and `AT_SYMLINK_FOLLOW` would then read as `AT_REMOVEDIR`. Roxy defines only the
 * flags it accepts: `AT_NO_AUTOMOUNT`, `AT_EMPTY_PATH`, and the `AT_STATX_*` family are absent.
 *
 * The kernel side is `kernel/syscall/src/syscalls/fs/mod.rs` (the selector) and
 * `kernel/syscall/src/syscalls/fs/dir.rs` (the flags).
 */

/* Operate on the working directory. Negative so it can never be a real descriptor, and not Linux's
 * -100, so that Linux's selector is reported as another personality's. */
#define AT_FDCWD (-0x200)

#define ROXY_AT_FLAGS_BASE (1 << 16)

/* Follow the final component if it is a symbolic link (the default; the flag is a no-op). */
#define AT_SYMLINK_NOFOLLOW ROXY_AT_FLAGS_BASE
/* Unlink a directory instead of the entry inside it. */
#define AT_REMOVEDIR (ROXY_AT_FLAGS_BASE << 1)
/* Follow the final component if it is a symbolic link. */
#define AT_SYMLINK_FOLLOW (ROXY_AT_FLAGS_BASE << 2)
/* Test using the effective user and group IDs. */
#define AT_EACCESS (ROXY_AT_FLAGS_BASE << 3)

#if defined(_GNU_SOURCE)
#define AT_NO_AUTOMOUNT 0x800
#define AT_EMPTY_PATH 0x1000
#endif

#if __MLIBC_LINUX_OPTION && defined(_GNU_SOURCE)

#define DN_ACCESS 1
#define DN_MODIFY 2
#define DN_CREATE 4
#define DN_DELETE 8
#define DN_RENAME 16
#define DN_ATTRIB 32
#define DN_MULTISHOT 0x80000000

#define AT_STATX_SYNC_AS_STAT 0x0000
#define AT_STATX_FORCE_SYNC 0x2000
#define AT_STATX_DONT_SYNC 0x4000
#define AT_STATX_SYNC_TYPE 0x6000
#define AT_RECURSIVE 0x8000

#endif /* __MLIBC_LINUX_OPTION && defined(_GNU_SOURCE) */

#if defined(_GNU_SOURCE) || __MLIBC_POSIX2024
struct f_owner_ex {
	int type;
	pid_t pid;
};
#endif /* defined(_GNU_SOURCE) || __MLIBC_POSIX2024 */

#define F_OWNER_TID 0
#define F_OWNER_PID 1
#define F_OWNER_PGRP 2

#define POSIX_FADV_NORMAL 0
#define POSIX_FADV_RANDOM 1
#define POSIX_FADV_SEQUENTIAL 2
#define POSIX_FADV_WILLNEED 3
#define POSIX_FADV_DONTNEED 4
#define POSIX_FADV_NOREUSE 5

#define S_IRWXU 0700
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXG 070
#define S_IRGRP 040
#define S_IWGRP 020
#define S_IXGRP 010
#define S_IRWXO 07
#define S_IROTH 04
#define S_IWOTH 02
#define S_IXOTH 01
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

#endif /* _ABIBITS_FCNTL_H */
