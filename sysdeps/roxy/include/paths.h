#ifndef _PATHS_H_
#define _PATHS_H_

/*
 * Conventional filesystem locations, following the BSD `paths.h` that every Unix
 * system provides: glibc, musl, and the BSDs all ship this header, and code as
 * ordinary as busybox's `libbb.h` includes it unconditionally.
 *
 * The entries name locations rather than promise that the file exists. Several of
 * them describe the accounting and database files a future Roxy may keep (passwd,
 * shadow, utmp, lastlog); consumers already treat a missing file as "no data".
 *
 * Two spellings follow Roxy's own layout rather than the classic ones: runtime
 * state lives in `/run`, and the editor is `/usr/bin/vim`.
 *
 * Paths that name *programs or devices Roxy does not provide* (csh, sendmail,
 * login, kernel memory devices, the kernel image) are deliberately absent: a port
 * that needs one should fail to build rather than exec a path that cannot exist.
 */

/* Default and standard utility search paths. Roxy installs no /sbin. */
#define _PATH_DEFPATH "/usr/bin:/bin"
#define _PATH_STDPATH "/usr/bin:/bin"

#define _PATH_BSHELL "/bin/sh"
#define _PATH_DEVNULL "/dev/null"
#define _PATH_CONSOLE "/dev/console"
#define _PATH_TTY "/dev/tty"
#define _PATH_DEV "/dev/"

#define _PATH_TMP "/tmp/"
#define _PATH_VARTMP "/var/tmp/"
#define _PATH_VARRUN "/run/"
#define _PATH_MAILDIR "/var/mail"
#define _PATH_MAN "/usr/share/man"
#define _PATH_VI "/usr/bin/vim"

#define _PATH_MNTTAB "/etc/fstab"
#define _PATH_MOUNTED "/etc/mtab"
#define _PATH_NOLOGIN "/etc/nologin"
#define _PATH_SHELLS "/etc/shells"
#define _PATH_PASSWD "/etc/passwd"
#define _PATH_GROUP "/etc/group"
#define _PATH_SHADOW "/etc/shadow"
#define _PATH_GSHADOW "/etc/gshadow"

#define _PATH_UTMP "/run/utmp"
#define _PATH_WTMP "/var/log/wtmp"
#define _PATH_LASTLOG "/var/log/lastlog"

#endif /* !_PATHS_H_ */
