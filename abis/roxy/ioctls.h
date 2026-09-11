#ifndef _ABIBITS_IOCTLS_H
#define _ABIBITS_IOCTLS_H

/*
 * Roxy's ioctl request numbers.
 *
 * This header diverges from `abis/linux/ioctls.h`, which only re-exports the Linux uapi headers
 * when the Linux option is enabled. Roxy owns its request numbers instead of borrowing Linux's,
 * so the numbers are defined here. They form a flat, global space in which every device family
 * owns a 0x100-aligned block, and no request number encodes a direction or a record size: a
 * number means one operation for the whole kernel. The kernel side is
 * `kernel/syscall/src/syscalls/ioctl/numbers.rs`; this header is the userspace half of the same
 * hand-maintained contract and changes with it.
 *
 * The names are the conventional Unix ones, so ported userspace compiles unchanged and keeps
 * calling the requests it expects; only the values are Roxy's. A request the kernel does not
 * implement is not defined here at all: `termios.h` and `sys/ioctl.h` keep their fallback
 * definitions for TIOCGSID, TIOCMGET/TIOCMBIS/TIOCMBIC, FIONREAD, FIONCLEX, FIOCLEX, and the
 * SIOC* requests, and the kernel answers those with ENOTTY.
 */

/* Terminal requests: TCGETS through TCFLSH. A family's base is its first request. */
#define ROXY_IOCTL_TERMINAL 0x0100u
#define TCGETS ROXY_IOCTL_TERMINAL
#define TCSETS (ROXY_IOCTL_TERMINAL + 1)
#define TCSETSW (ROXY_IOCTL_TERMINAL + 2)
#define TCSETSF (ROXY_IOCTL_TERMINAL + 3)
#define TIOCGWINSZ (ROXY_IOCTL_TERMINAL + 4)
#define TIOCSWINSZ (ROXY_IOCTL_TERMINAL + 5)
#define TIOCGPGRP (ROXY_IOCTL_TERMINAL + 6)
#define TIOCSPGRP (ROXY_IOCTL_TERMINAL + 7)
#define TIOCSCTTY (ROXY_IOCTL_TERMINAL + 8)
#define TCFLSH (ROXY_IOCTL_TERMINAL + 9)

/* Pseudo-terminal requests: the slave number and its lock. */
#define ROXY_IOCTL_PTY 0x0200u
#define TIOCGPTN ROXY_IOCTL_PTY
#define TIOCSPTLCK (ROXY_IOCTL_PTY + 1)

/* Framebuffer requests; the family's own header names them. */
#define ROXY_IOCTL_FRAMEBUFFER 0x0300u

/* Requests that act on the open file description rather than on a device. */
#define ROXY_IOCTL_DESCRIPTION 0x0400u
#define FIONBIO ROXY_IOCTL_DESCRIPTION

#endif /* _ABIBITS_IOCTLS_H */
