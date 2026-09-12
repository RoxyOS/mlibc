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
 * The fcntl command space.
 *
 * Commands carry Roxy's own numbers, laid out from ROXY_F_CMD_BASE so that a command below the
 * base cannot be one of ours: a program compiled against a foreign numbering (Linux's F_GETFD
 * is 1, F_DUPFD_CLOEXEC is 1030) hands the kernel a value it recognizes and reports, instead of
 * one that silently aliases an unrelated Roxy command.
 *
 * Every command this kernel does not implement is defined to F_UNSUPPORTED, which keeps ported
 * sources compiling while making the call unmissable at runtime. Because they all share one
 * value, two unsupported commands cannot appear as distinct `case` labels in a switch; no ported
 * source needs that, since none of them is served.
 *
 * The kernel side of this contract is kernel/syscall/src/syscalls/fcntl.rs.
 */
#define ROXY_F_CMD_BASE 0x1000u
#define F_UNSUPPORTED 0x100u

/* Commands the kernel implements; a value is the base plus the command's index. */
#define F_DUPFD (ROXY_F_CMD_BASE + 0)
#define F_GETFD (ROXY_F_CMD_BASE + 1)
#define F_SETFD (ROXY_F_CMD_BASE + 2)
#define F_GETFL (ROXY_F_CMD_BASE + 3)
#define F_SETFL (ROXY_F_CMD_BASE + 4)
#define F_DUPFD_CLOEXEC (ROXY_F_CMD_BASE + 5)

/* Commands the kernel does not implement. The `64` spellings are kept because
   ported sources name them explicitly. */
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

/* Argument bit of the supported F_GETFD/F_SETFD. */
#define FD_CLOEXEC 1

#define AT_FDCWD -100
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200
#define AT_SYMLINK_FOLLOW 0x400
#define AT_EACCESS 0x200

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
