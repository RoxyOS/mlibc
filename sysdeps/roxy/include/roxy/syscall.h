#ifndef _ROXY_SYSCALL_H
#define _ROXY_SYSCALL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long roxy_syscall_word_t;

/*
 * Roxy's syscall numbers.
 *
 * A number is `ROXY_SYS_BASE` plus the syscall's index, so the whole space sits above every
 * number another personality uses for the syscalls this kernel provides: a number below the base
 * is never a Roxy one, and the kernel reports a caller that issued one as foreign instead of
 * misreading it as a syscall of its own. That is what a program with its own syscall layer does —
 * a language runtime issuing `syscall` directly with Linux's numbers. The low range also stays
 * free for a Linux-compatible personality to serve unshifted.
 *
 * The kernel side is `kernel/syscall/src/numbers.rs`; this header is the userspace half of the
 * same hand-maintained contract and changes with it. Only this library reads these numbers, so a
 * change here does not reach a consumer's object files the way an `abi-bits/` macro does.
 */
#define ROXY_SYS_BASE 0x1000

#define ROXY_SYS_EXIT (ROXY_SYS_BASE + 0)
#define ROXY_SYS_READ (ROXY_SYS_BASE + 1)
#define ROXY_SYS_WRITE (ROXY_SYS_BASE + 2)
#define ROXY_SYS_FUTEX_WAIT (ROXY_SYS_BASE + 3)
#define ROXY_SYS_FUTEX_WAKE (ROXY_SYS_BASE + 4)
#define ROXY_SYS_ANON_ALLOCATE (ROXY_SYS_BASE + 5)
#define ROXY_SYS_ANON_FREE (ROXY_SYS_BASE + 6)
#define ROXY_SYS_TCB_SET (ROXY_SYS_BASE + 7)
#define ROXY_SYS_CLOCK_GET (ROXY_SYS_BASE + 8)
#define ROXY_SYS_VM_MAP (ROXY_SYS_BASE + 9)
#define ROXY_SYS_VM_UNMAP (ROXY_SYS_BASE + 10)
#define ROXY_SYS_CLOSE (ROXY_SYS_BASE + 11)
#define ROXY_SYS_SEEK (ROXY_SYS_BASE + 12)
#define ROXY_SYS_ISATTY (ROXY_SYS_BASE + 13)
#define ROXY_SYS_OPEN (ROXY_SYS_BASE + 14)
#define ROXY_SYS_VM_PROTECT (ROXY_SYS_BASE + 15)
#define ROXY_SYS_STAT (ROXY_SYS_BASE + 16)
#define ROXY_SYS_FORK (ROXY_SYS_BASE + 17)
#define ROXY_SYS_EXECVE (ROXY_SYS_BASE + 18)
#define ROXY_SYS_GETPID (ROXY_SYS_BASE + 19)
#define ROXY_SYS_GETPPID (ROXY_SYS_BASE + 20)
#define ROXY_SYS_GETEUID (ROXY_SYS_BASE + 21)
#define ROXY_SYS_GETUID (ROXY_SYS_BASE + 22)
#define ROXY_SYS_GETGID (ROXY_SYS_BASE + 23)
#define ROXY_SYS_GETEGID (ROXY_SYS_BASE + 24)
#define ROXY_SYS_WAITPID (ROXY_SYS_BASE + 25)
#define ROXY_SYS_SIGPROCMASK (ROXY_SYS_BASE + 26)
#define ROXY_SYS_SIGACTION (ROXY_SYS_BASE + 27)
#define ROXY_SYS_OPEN_DIR (ROXY_SYS_BASE + 28)
#define ROXY_SYS_READ_ENTRIES (ROXY_SYS_BASE + 29)
#define ROXY_SYS_CHDIR (ROXY_SYS_BASE + 30)
#define ROXY_SYS_IOCTL (ROXY_SYS_BASE + 31)
#define ROXY_SYS_GETCWD (ROXY_SYS_BASE + 32)
#define ROXY_SYS_POLL (ROXY_SYS_BASE + 33)
#define ROXY_SYS_SLEEP (ROXY_SYS_BASE + 34)
#define ROXY_SYS_SEND_SIGNAL (ROXY_SYS_BASE + 35)
#define ROXY_SYS_PPOLL (ROXY_SYS_BASE + 36)
#define ROXY_SYS_PSELECT (ROXY_SYS_BASE + 37)
#define ROXY_SYS_UNAME (ROXY_SYS_BASE + 38)
#define ROXY_SYS_MKDIRAT (ROXY_SYS_BASE + 39)
#define ROXY_SYS_UNLINKAT (ROXY_SYS_BASE + 40)
#define ROXY_SYS_READLINKAT (ROXY_SYS_BASE + 41)
#define ROXY_SYS_LINKAT (ROXY_SYS_BASE + 42)
#define ROXY_SYS_SYMLINKAT (ROXY_SYS_BASE + 43)
#define ROXY_SYS_RENAMEAT (ROXY_SYS_BASE + 44)
#define ROXY_SYS_SYNC (ROXY_SYS_BASE + 45)
#define ROXY_SYS_FSYNC (ROXY_SYS_BASE + 46)
#define ROXY_SYS_FTRUNCATE (ROXY_SYS_BASE + 47)
#define ROXY_SYS_SOCKETPAIR (ROXY_SYS_BASE + 48)
#define ROXY_SYS_SOCKET (ROXY_SYS_BASE + 49)
#define ROXY_SYS_BIND (ROXY_SYS_BASE + 50)
#define ROXY_SYS_LISTEN (ROXY_SYS_BASE + 51)
#define ROXY_SYS_ACCEPT (ROXY_SYS_BASE + 52)
#define ROXY_SYS_CONNECT (ROXY_SYS_BASE + 53)
#define ROXY_SYS_SOCKNAME (ROXY_SYS_BASE + 61)
#define ROXY_SYS_PEERNAME (ROXY_SYS_BASE + 62)
#define ROXY_SYS_SHUTDOWN (ROXY_SYS_BASE + 63)
#define ROXY_SYS_GETSOCKOPT (ROXY_SYS_BASE + 64)
#define ROXY_SYS_ACCESS (ROXY_SYS_BASE + 65)
#define ROXY_SYS_RECVMSG (ROXY_SYS_BASE + 66)
#define ROXY_SYS_SENDMSG (ROXY_SYS_BASE + 67)
#define ROXY_SYS_PIPE (ROXY_SYS_BASE + 55)
#define ROXY_SYS_DUP2 (ROXY_SYS_BASE + 56)
#define ROXY_SYS_FCNTL (ROXY_SYS_BASE + 57)
#define ROXY_SYS_UMASK (ROXY_SYS_BASE + 58)
#define ROXY_SYS_CHMOD (ROXY_SYS_BASE + 59)
#define ROXY_SYS_FCHMOD (ROXY_SYS_BASE + 60)
#define ROXY_SYS_SET_PGID (ROXY_SYS_BASE + 68)
#define ROXY_SYS_GET_PGID (ROXY_SYS_BASE + 69)
#define ROXY_SYS_SET_SID (ROXY_SYS_BASE + 70)
#define ROXY_SYS_WRITEV (ROXY_SYS_BASE + 71)
#define ROXY_SYS_TTYNAME (ROXY_SYS_BASE + 72)
#define ROXY_SYS_TIMER_CREATE (ROXY_SYS_BASE + 73)
#define ROXY_SYS_TIMER_SETTIME (ROXY_SYS_BASE + 74)
#define ROXY_SYS_TIMER_GETTIME (ROXY_SYS_BASE + 75)
#define ROXY_SYS_TIMER_GETOVERRUN (ROXY_SYS_BASE + 76)
#define ROXY_SYS_TIMER_DELETE (ROXY_SYS_BASE + 77)
#define ROXY_SYS_THREAD_CREATE (ROXY_SYS_BASE + 78)
#define ROXY_SYS_THREAD_EXIT (ROXY_SYS_BASE + 79)
#define ROXY_SYS_GET_TID (ROXY_SYS_BASE + 80)
#define ROXY_SYS_SIGTIMEDWAIT (ROXY_SYS_BASE + 81)
#define ROXY_SYS_TGKILL (ROXY_SYS_BASE + 82)
#define ROXY_SYS_CLOCK_GETRES (ROXY_SYS_BASE + 83)

typedef struct {
	int64_t seconds;
	int64_t nanoseconds;
} roxy_clock_result;

typedef struct {
	uint64_t file_id;
	uint64_t size;
	uint64_t blocks;
	uint64_t hard_links;
	uint32_t mode;
	uint32_t block_size;
} roxy_stat_result;

typedef struct {
	uint64_t inode;
	int64_t offset;
	uint16_t record_size;
	uint8_t type;
	char name[256];
	uint8_t padding[5];
} roxy_dirent;

#ifdef __cplusplus
static_assert(sizeof(roxy_clock_result) == 16);
static_assert(alignof(roxy_clock_result) == 8);
static_assert(offsetof(roxy_clock_result, seconds) == 0);
static_assert(offsetof(roxy_clock_result, nanoseconds) == 8);
static_assert(sizeof(roxy_stat_result) == 40);
static_assert(alignof(roxy_stat_result) == 8);
static_assert(offsetof(roxy_stat_result, file_id) == 0);
static_assert(offsetof(roxy_stat_result, size) == 8);
static_assert(offsetof(roxy_stat_result, blocks) == 16);
static_assert(offsetof(roxy_stat_result, hard_links) == 24);
static_assert(offsetof(roxy_stat_result, mode) == 32);
static_assert(offsetof(roxy_stat_result, block_size) == 36);
static_assert(sizeof(roxy_dirent) == 280);
static_assert(alignof(roxy_dirent) == 8);
static_assert(offsetof(roxy_dirent, inode) == 0);
static_assert(offsetof(roxy_dirent, offset) == 8);
static_assert(offsetof(roxy_dirent, record_size) == 16);
static_assert(offsetof(roxy_dirent, type) == 18);
static_assert(offsetof(roxy_dirent, name) == 19);
#endif

roxy_syscall_word_t roxy_syscall0(long number);

roxy_syscall_word_t roxy_syscall1(long number, roxy_syscall_word_t first);

roxy_syscall_word_t roxy_syscall2(
	long number,
	roxy_syscall_word_t first,
	roxy_syscall_word_t second
);

roxy_syscall_word_t roxy_syscall3(
	long number,
	roxy_syscall_word_t first,
	roxy_syscall_word_t second,
	roxy_syscall_word_t third
);

roxy_syscall_word_t roxy_syscall4(
	long number,
	roxy_syscall_word_t first,
	roxy_syscall_word_t second,
	roxy_syscall_word_t third,
	roxy_syscall_word_t fourth
);

roxy_syscall_word_t roxy_syscall5(
	long number,
	roxy_syscall_word_t first,
	roxy_syscall_word_t second,
	roxy_syscall_word_t third,
	roxy_syscall_word_t fourth,
	roxy_syscall_word_t fifth
);

roxy_syscall_word_t roxy_syscall6(
	long number,
	roxy_syscall_word_t first,
	roxy_syscall_word_t second,
	roxy_syscall_word_t third,
	roxy_syscall_word_t fourth,
	roxy_syscall_word_t fifth,
	roxy_syscall_word_t sixth
);

#ifdef __cplusplus
}
#endif

#endif
