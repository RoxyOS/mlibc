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
 * SIOC* requests, and the kernel reports those through its unsupported diagnostic and answers
 * with ENOTTY.
 *
 * The whole space is one narrow window above `ROXY_IOCTL_BASE`. The base sits above every number
 * Linux produces for the requests this kernel serves: Linux's plain `_IO(type, nr)` form caps at
 * 0xFFFF, and its `_IOW`/`_IOR`/`_IOWR` forms set a direction bit far above the window. A request
 * below the base is therefore never a Roxy one, and the kernel reports it as a foreign request
 * instead of misreading it as an unrelated operation of its own, which is what a program
 * hardcoding a Linux constant would otherwise get.
 */

#define ROXY_IOCTL_BASE 0x10000u

/* Terminal requests: TCGETS through TCFLSH. A family's base is its first request, and families
   follow one another in 0x100-aligned blocks. */
#define ROXY_IOCTL_TERMINAL (ROXY_IOCTL_BASE + 0x000u)
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
#define ROXY_IOCTL_PTY (ROXY_IOCTL_BASE + 0x100u)
#define TIOCGPTN ROXY_IOCTL_PTY
#define TIOCSPTLCK (ROXY_IOCTL_PTY + 1)

/* Framebuffer requests; the family's own header names them. */
#define ROXY_IOCTL_FRAMEBUFFER (ROXY_IOCTL_BASE + 0x200u)

/* Requests that act on the open file description rather than on a device. */
#define ROXY_IOCTL_DESCRIPTION (ROXY_IOCTL_BASE + 0x300u)
#define FIONBIO ROXY_IOCTL_DESCRIPTION

#endif /* _ABIBITS_IOCTLS_H */
