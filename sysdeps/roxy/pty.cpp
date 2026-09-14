#include <errno.h>
#include <pty.h>
#include <roxy/syscall.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace {

// Applies the caller's attributes to the slave, which owns the termios and window size of a Roxy
// pty pair: the master is a dumb byte stream with no terminal attributes of its own.
bool apply_attributes(
	int slave,
	const struct termios *attributes,
	const struct winsize *window_size
) {
	if (attributes && tcsetattr(slave, TCSANOW, attributes) != 0)
		return false;
	if (window_size && ioctl(slave, TIOCSWINSZ, window_size) != 0)
		return false;
	return true;
}

} // namespace

int openpty(
	int *mfd,
	int *sfd,
	char *name,
	const struct termios *attributes,
	const struct winsize *window_size
) {
	// A Roxy pair has no device-filesystem name, so there is nothing to report.
	(void)name;

	int descriptors[2] = {-1, -1};
	auto result = roxy_syscall1(ROXY_SYS_OPENPTY, reinterpret_cast<long>(descriptors));
	if (result.error) {
		errno = static_cast<int>(result.error);
		return -1;
	}

	// Apply the optional attributes before handing the pair over, so a rejected attribute never
	// leaves a half-configured pair open behind the caller's back.
	if (!apply_attributes(descriptors[1], attributes, window_size)) {
		int saved_error = errno;
		close(descriptors[0]);
		close(descriptors[1]);
		errno = saved_error;
		return -1;
	}

	*mfd = descriptors[0];
	*sfd = descriptors[1];
	return 0;
}
