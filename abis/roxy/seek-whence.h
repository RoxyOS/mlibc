#ifndef _ABIBITS_SEEK_WHENCE_H
#define _ABIBITS_SEEK_WHENCE_H

/*
 * Roxy's `whence` enumeration for `lseek`.
 *
 * Values are numbered from `ROXY_SEEK_BASE`, which sits above Linux's range, so a `whence` below
 * the base is another personality's numbering: the kernel reports such a caller as foreign
 * instead of reading it as a request of its own. Linux's `SEEK_DATA` and `SEEK_HOLE` are not
 * defined here — the kernel implements neither.
 *
 * The kernel side is `kernel/syscall/src/syscalls/seek.rs`.
 */
#define ROXY_SEEK_BASE 0x100

/* Seek from the start of the file. */
#define SEEK_SET ROXY_SEEK_BASE
/* Seek from the current position. */
#define SEEK_CUR (ROXY_SEEK_BASE + 1)
/* Seek from the end of the file. */
#define SEEK_END (ROXY_SEEK_BASE + 2)

#endif /* _ABIBITS_SEEK_WHENCE_H */
