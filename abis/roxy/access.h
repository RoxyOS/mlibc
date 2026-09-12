#ifndef _ABIBITS_ACCESS_H
#define _ABIBITS_ACCESS_H

/*
 * Roxy's `access` mode word.
 *
 * Each right is one bit, and the lowest one is `ROXY_ACCESS_BASE`. Linux's modes are `F_OK` 0,
 * `X_OK` 1, `W_OK` 2, and `R_OK` 4, so a word below the base is another personality's numbering:
 * the kernel reports a caller that passes one as foreign instead of reading it as a request of its
 * own. Giving every right its own bit, rather than one base shared by all of them, also keeps a
 * caller's `&` test for one right from answering true because another was requested.
 *
 * The kernel side is `kernel/syscall/src/syscalls/access.rs`.
 */
#define ROXY_ACCESS_BASE 0x100

/* Test for existence only. */
#define F_OK ROXY_ACCESS_BASE
/* Test for execute/search permission. */
#define X_OK (ROXY_ACCESS_BASE << 1)
/* Test for write permission. */
#define W_OK (ROXY_ACCESS_BASE << 2)
/* Test for read permission. */
#define R_OK (ROXY_ACCESS_BASE << 3)

#endif /* _ABIBITS_ACCESS_H */
