#include <errno.h>
#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>
#include <stdio.h>
#include <termios.h>

namespace mlibc {

namespace {

constexpr unsigned long tcgets = 0x5401;
constexpr unsigned long tcsets = 0x5402;
constexpr unsigned long tcsetsw = 0x5403;
constexpr unsigned long tcsetsf = 0x5404;
constexpr unsigned long tiocgwinsz = 0x5413;
constexpr unsigned long tiocswinsz = 0x5414;
// Linux TIOCGPTN/TIOCSPTLCK, matching kernel/syscall/.../ioctl/pty.rs and the Roxy ABI.
constexpr unsigned long tiocgptn = 0x80045430;   // get pty slave number into the argument
constexpr unsigned long tiocsptlck = 0x40045431; // lock/unlock pty slave (0 unlocks)
constexpr unsigned long tcflush = 0x540b;         // flush queued terminal input/output

int terminal_ioctl(int fd, unsigned long request, void *argument) {
	int output;

	return sysdep<Ioctl>(fd, request, argument, &output);
}

} // namespace

int Sysdeps<Ioctl>::operator()(int fd, unsigned long request, void *argument, int *output) {
	auto result = roxy_syscall3(
	    ROXY_SYS_IOCTL,
	    fd,
	    static_cast<long>(request),
	    reinterpret_cast<long>(argument)
	);
	if(result < 0)
		return static_cast<int>(-result);

	if(output)
		*output = static_cast<int>(result);
	return 0;
}

int Sysdeps<Tcgetattr>::operator()(int fd, struct termios *attributes) {
	return terminal_ioctl(fd, tcgets, attributes);
}

int Sysdeps<Tcsetattr>::operator()(
	int fd,
	int optional_action,
	const struct termios *attributes
) {
	unsigned long request;

	switch(optional_action) {
		case TCSANOW: request = tcsets; break;
		case TCSADRAIN: request = tcsetsw; break;
		case TCSAFLUSH: request = tcsetsf; break;
		default: return EINVAL;
	}

	return terminal_ioctl(fd, request, const_cast<struct termios *>(attributes));
}

int Sysdeps<Tcgetwinsize>::operator()(int fd, struct winsize *window_size) {
	return terminal_ioctl(fd, tiocgwinsz, window_size);
}

int Sysdeps<Tcsetwinsize>::operator()(int fd, const struct winsize *window_size) {
	return terminal_ioctl(fd, tiocswinsz, const_cast<struct winsize *>(window_size));
}

int Sysdeps<Tcflush>::operator()(int fd, int queue_selector) {
	// TCFLSH carries the queue selector (TCIFLUSH/TCOFLUSH/TCIOFLUSH) by value, not as a pointer.
	return terminal_ioctl(fd, tcflush, reinterpret_cast<void *>(static_cast<long>(queue_selector)));
}

int Sysdeps<Ptsname>::operator()(int fd, char *buffer, size_t length) {
	unsigned int number = 0;
	if (int error = terminal_ioctl(fd, tiocgptn, &number))
		return error;

	int written = snprintf(buffer, length, "/dev/pts/%u", number);
	if (written < 0)
		return errno ? errno : EIO;
	if (static_cast<size_t>(written) >= length)
		return ERANGE;
	return 0;
}

int Sysdeps<Unlockpt>::operator()(int fd) {
	int unlock = 0;
	return terminal_ioctl(fd, tiocsptlck, &unlock);
}

} // namespace mlibc
