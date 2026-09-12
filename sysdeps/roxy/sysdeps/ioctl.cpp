#include <errno.h>
#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

namespace mlibc {

namespace {

// Request numbers come from <sys/ioctl.h> -> <abi-bits/ioctls.h>, the userspace half of the
// request-number contract with the kernel (kernel/syscall/src/syscalls/ioctl/numbers.rs).

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
	return terminal_ioctl(fd, TCGETS, attributes);
}

int Sysdeps<Tcsetattr>::operator()(
	int fd,
	int optional_action,
	const struct termios *attributes
) {
	unsigned long request;

	switch(optional_action) {
		case TCSANOW: request = TCSETS; break;
		case TCSADRAIN: request = TCSETSW; break;
		case TCSAFLUSH: request = TCSETSF; break;
		default: return EINVAL;
	}

	return terminal_ioctl(fd, request, const_cast<struct termios *>(attributes));
}

int Sysdeps<Tcgetwinsize>::operator()(int fd, struct winsize *window_size) {
	return terminal_ioctl(fd, TIOCGWINSZ, window_size);
}

int Sysdeps<Tcsetwinsize>::operator()(int fd, const struct winsize *window_size) {
	return terminal_ioctl(fd, TIOCSWINSZ, const_cast<struct winsize *>(window_size));
}

int Sysdeps<Tcflush>::operator()(int fd, int queue_selector) {
	// TCFLSH carries the queue selector (TCIFLUSH/TCOFLUSH/TCIOFLUSH) by value, not as a pointer.
	return terminal_ioctl(fd, TCFLSH, reinterpret_cast<void *>(static_cast<long>(queue_selector)));
}

int Sysdeps<Ptsname>::operator()(int fd, char *buffer, size_t length) {
	unsigned int number = 0;
	if (int error = terminal_ioctl(fd, TIOCGPTN, &number))
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
	return terminal_ioctl(fd, TIOCSPTLCK, &unlock);
}

} // namespace mlibc
