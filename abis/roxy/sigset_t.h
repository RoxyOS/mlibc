#ifndef _ABIBITS_SIGSET_T_H
#define _ABIBITS_SIGSET_T_H

/*
 * Roxy's `sigset_t`: one word, one bit per signal.
 *
 * The set is the value the kernel carries rather than a struct wrapping one, so no member stands
 * between the two sides and no tail reserves bits that carry nothing. One word is exactly enough:
 * this ABI numbers signals 1..64.
 *
 * The kernel side is `kernel/syscall/src/syscalls/signal/mod.rs`, where the same word asserts its
 * size and alignment as the boundary contract.
 */
typedef unsigned long sigset_t;

#endif /* _ABIBITS_SIGSET_T_H */
