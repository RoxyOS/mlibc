#ifndef _ABIBITS_SIGNAL_H
#define _ABIBITS_SIGNAL_H

#include <mlibc-config.h>

#include <abi-bits/pid_t.h>
#include <abi-bits/sigval.h>
#include <abi-bits/sigset_t.h>
#include <abi-bits/uid_t.h>
#include <bits/ansi/clock_t.h>
#include <bits/size_t.h>
#include <bits/types.h>

#if __MLIBC_POSIX_OPTION
#include <abi-bits/sigevent.h>
#endif

#if defined(_DEFAULT_SOURCE) || (__MLIBC_POSIX1 && !__MLIBC_POSIX2024)
#define POLL_IN 1
#define POLL_OUT 2
#define POLL_MSG 3
#define POLL_ERR 4
#define POLL_PRI 5
#define POLL_HUP 6
#endif

/*
 * The information record handed to a signal handler.
 *
 * The record is flat rather than a union overlay: a source fills the fields it has, so every
 * member's offset is a constant instead of an overlay that `si_code` selects. Members keep their
 * POSIX names, so ported handlers compile unchanged. `si_overrun` and `si_value` keep the offsets
 * the Linux-shaped record gave them.
 *
 * The kernel side is `kernel/process/src/signal_frame`, which builds the record written on a
 * signal frame.
 */
typedef struct {
	int si_signo;
	int si_code;
	pid_t si_pid;
	uid_t si_uid;
	int si_status;
	int si_overrun;
	union sigval si_value;
	void *si_addr;
} siginfo_t;

#ifdef __cplusplus
/* Contract with the kernel signal-frame writer: the `siginfo_t` layout must stay byte-for-byte
   compatible with kernel/process/src/signal_frame, whose own assertions pin the same offsets. The
   asserted values are LP64 (x86_64) offsets, which is the only ABI Roxy targets. */
#if defined(__x86_64__)
static_assert(sizeof(siginfo_t) == 40);
static_assert(__builtin_offsetof(siginfo_t, si_pid) == 8);
static_assert(__builtin_offsetof(siginfo_t, si_overrun) == 20);
static_assert(__builtin_offsetof(siginfo_t, si_value) == 24);
static_assert(__builtin_offsetof(siginfo_t, si_addr) == 32);
#endif
#endif

/* Required for sys_sigaction sysdep. */
#define SA_NOCLDSTOP 1
#define SA_NOCLDWAIT 2
#define SA_SIGINFO 4
#define SA_ONSTACK 0x08000000
#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000
#define SA_RESTORER 0x04000000

/* SA_NOMASK is an alias for SA_NODEFER */
/* SA_ONESHOT is an alias for SA_RESETHAND */
#define SA_NOMASK SA_NODEFER
#define SA_ONESHOT SA_RESETHAND

#ifdef __cplusplus
extern "C" {
#endif

/* Argument for signal() */
typedef void (*__sighandler) (int);

#define SIG_ERR ((__sighandler)(void *)(-1))
#define SIG_DFL ((__sighandler)(void *)(0))
#define SIG_IGN ((__sighandler)(void *)(1))

#define SIGABRT 6
#define SIGFPE 8
#define SIGILL 4
#define SIGINT 2
#define SIGSEGV 11
#define SIGTERM 15
#define SIGPROF 27
#define SIGIO 29
#define SIGPWR 30
#define SIGRTMIN 35
#define SIGRTMAX 64

/*
 * Roxy's `sigprocmask` operations.
 *
 * Values are numbered from `ROXY_MASK_HOW_BASE`, above Linux's range, so a `how` below the base is
 * another personality's numbering and the kernel reports it as foreign.
 *
 * The kernel side is `kernel/syscall/src/syscalls/signal/mask.rs`.
 */
#define ROXY_MASK_HOW_BASE 0x100
#define SIG_BLOCK ROXY_MASK_HOW_BASE
#define SIG_UNBLOCK (ROXY_MASK_HOW_BASE + 1)
#define SIG_SETMASK (ROXY_MASK_HOW_BASE + 2)

#define SIGHUP    1
#define SIGQUIT   3
#define SIGTRAP   5
#define SIGIOT    SIGABRT
#define SIGBUS    7
#define SIGKILL   9
#define SIGUSR1   10
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGSTKFLT 16
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22
#define SIGURG    23
#define SIGXCPU   24
#define SIGXFSZ   25
#define SIGVTALRM 26
#define SIGWINCH  28
#define SIGPOLL   29
#define SIGSYS    31
#define SIGUNUSED SIGSYS
#define SIGCANCEL 32
#define SIGTIMER  33

#if __MLIBC_XOPEN

#if defined(__x86_64__) || defined(__i386__) || defined(__riscv) || defined(__m68k__)
#define MINSIGSTKSZ 2048
#define SIGSTKSZ 8192
#elif defined(__aarch64__)
#define MINSIGSTKSZ 5120
#define SIGSTKSZ 16384
#elif defined(__loongarch64)
#define MINSIGSTKSZ 4096
#define SIGSTKSZ 16384
#else
#error unhandled architecture
#endif

#define SS_ONSTACK 1
#define SS_DISABLE 2
#endif

typedef struct __stack {
	void *ss_sp;
	int ss_flags;
	size_t ss_size;
} stack_t;

/*
 * Roxy's `sigevent.sigev_notify` values.
 *
 * Numbered as a small enumeration from `ROXY_SIGEV_BASE`, above Linux's range, so a
 * `sigev_notify` below the base is another personality's numbering and the kernel reports it as
 * foreign.
 *
 * The kernel side is `kernel/syscall/src/syscalls/timer/abi.rs`.
 */
#define ROXY_SIGEV_BASE 0x100
#define SIGEV_SIGNAL ROXY_SIGEV_BASE
#define SIGEV_NONE (ROXY_SIGEV_BASE + 1)
#define SIGEV_THREAD (ROXY_SIGEV_BASE + 2)
#define SIGEV_THREAD_ID (ROXY_SIGEV_BASE + 3)

#define SEGV_MAPERR 1
#define SEGV_ACCERR 2

#define BUS_ADRALN 1
#define BUS_ADRERR 2
#define BUS_OBJERR 3
#define BUS_MCEERR_AR 4
#define BUS_MCEERR_AO 5

#define ILL_ILLOPC 1
#define ILL_ILLOPN 2
#define ILL_ILLADR 3
#define ILL_ILLTRP 4
#define ILL_PRVOPC 5
#define ILL_PRVREG 6
#define ILL_COPROC 7
#define ILL_BADSTK 8
#define ILL_BADIADDR 9

#define _NSIG 65
#if defined(_DEFAULT_SOURCE)
#define NSIG _NSIG
#endif

#define SI_ASYNCNL (-60)
#define SI_TKILL (-6)
#define SI_SIGIO (-5)
#define SI_ASYNCIO (-4)
#define SI_MESGQ (-3)
#define SI_TIMER (-2)
#define SI_QUEUE (-1)
#define SI_USER 0
#define SI_KERNEL 128

#include <bits/threads.h>

struct sigaction {
	union {
		void (*sa_handler)(int);
		void (*sa_sigaction)(int, siginfo_t *, void *);
	} __sa_handler;
	int sa_flags;
	void (*sa_restorer)(void);
	sigset_t sa_mask;
};

#define sa_handler __sa_handler.sa_handler
#define sa_sigaction __sa_handler.sa_sigaction

/*
 * The machine-context record an `SA_SIGINFO` handler receives a pointer to.
 *
 * Roxy defines no machine context: the kernel passes a null third argument, because it serves no
 * state for a handler to inspect or redirect, and it declares no `mcontext_t` or `REG_*` indices to
 * read one with. `ucontext.h` and mlibc's cancellation path name this type, so it exists for them
 * to compile against; it is deliberately opaque, and nothing in Roxy produces or interprets one.
 *
 * TODO(signal-handler-context): serving that argument means defining the interrupted machine state
 * here and having the kernel record it on the signal frame.
 */
typedef struct {
	unsigned long __reserved;
} ucontext_t;

#ifdef __cplusplus
}
#endif

#endif /* _ABIBITS_SIGNAL_H */
