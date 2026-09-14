#ifndef _PTY_H
#define _PTY_H

/*
 * Roxy's pseudo-terminal allocation.
 *
 * Roxy has no `/dev/ptmx` and no `/dev/pts` directory: a pty pair is an anonymous descriptor pair,
 * so the Unix98 `posix_openpt`/`grantpt`/`unlockpt`/`ptsname` family has no backing on this
 * platform and this header does not declare it. `openpty` is the only way to allocate a pair, and
 * it returns the slave descriptor directly, so a caller that needs the slave in a child process
 * passes the descriptor across `fork` and makes it the child's controlling terminal with
 * `TIOCSCTTY` (the `login_tty`-style setup) rather than reopening a device path.
 *
 * `name`, when non-NULL, is left unwritten: a Roxy pair has no device-filesystem name to report,
 * and no path exists that a caller could reopen.
 */

#include <mlibc-config.h>
#include <termios.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MLIBC_ABI_ONLY

int openpty(
	int *__mfd,
	int *__sfd,
	char *__name,
	const struct termios *__ios,
	const struct winsize *__win
);

#endif /* !__MLIBC_ABI_ONLY */

#ifdef __cplusplus
}
#endif

#endif /* _PTY_H */
