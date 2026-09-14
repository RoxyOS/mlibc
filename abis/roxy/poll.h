#ifndef _ABIBITS_POLL_H
#define _ABIBITS_POLL_H

/*
 * Roxy's poll event word keeps the POSIX condition names it serves and no others.
 *
 * `POLLRDNORM`, `POLLRDBAND`, `POLLRDHUP`, `POLLWRNORM`, and `POLLWRBAND` named conditions the
 * kernel does not report; leaving them defined would let a program ask for one and wait for it
 * forever. A name this header does not define makes asking for one a compile error instead.
 *
 * The remaining bits keep the numbering the POSIX names have, because `struct pollfd.events` is
 * upstream mlibc's `short`; the kernel takes the conditions in Roxy's own request word, which this
 * library translates to and from.
 */
#define POLLIN 0x0001
#define POLLPRI 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define POLLNVAL 0x0020

#endif /* _ABIBITS_POLL_H */
