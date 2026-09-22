#include <stddef.h>
#include <stdint.h>

#include <errno.h>
#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>
#include <sys/ioctl.h>
#include <termios.h>

/*
 * Roxy's terminal-attribute record.
 *
 * `TCGETS`/`TCSETS*` carry this record rather than the POSIX `struct termios`, which stays a
 * library-visible type that `tcgetattr`/`tcsetattr` translate to and from. The record carries
 * exactly the attributes a Roxy terminal acts on, so a POSIX attribute with no field here is
 * dropped by this translation (see the TODO in termios_to_roxy_term_attr). The kernel side is
 * `kernel/syscall/src/syscalls/ioctl/terminal_abi.rs`; this file is the other half of the same
 * hand-maintained contract and both sides change together.
 */

/* Terminal behavior flags of `roxy_terminal_attributes.flags`.
 *
 * Each flag owns one bit. The word is a field of Roxy's own record rather than a word a caller
 * shares with another personality, so it has no base above a foreign numbering: a bit outside
 * this set is undefined and the kernel reports it.
 */
#define ROXY_TERM_ISIG (1u << 0)
#define ROXY_TERM_ICANON (1u << 1)
#define ROXY_TERM_ECHO (1u << 2)
#define ROXY_TERM_OPOST (1u << 3)
#define ROXY_TERM_ONLCR (1u << 4)
#define ROXY_TERM_ICRNL (1u << 5)
#define ROXY_TERM_INLCR (1u << 6)
#define ROXY_TERM_IGNCR (1u << 7)

namespace mlibc {

namespace {

// Request numbers come from <sys/ioctl.h> -> <abi-bits/ioctls.h>, the userspace half of the
// request-number contract with the kernel (kernel/syscall/src/syscalls/ioctl/numbers.rs).

/* The terminal attributes a Roxy terminal applies. `flags` is one `ROXY_TERM_*` bit per
 * attribute; `interrupt_byte` and `erase_byte` are the only control characters a Roxy terminal
 * acts on. `reserved` is always zero and keeps the record free of implicit padding.
 */
typedef struct {
	uint32_t flags;
	uint8_t interrupt_byte;
	uint8_t erase_byte;
	uint16_t reserved;
} roxy_terminal_attributes;

static_assert(sizeof(roxy_terminal_attributes) == 8,
              "roxy_terminal_attributes is an 8-byte wire record");
static_assert(alignof(roxy_terminal_attributes) == 4,
              "roxy_terminal_attributes is 4-byte aligned");
static_assert(offsetof(roxy_terminal_attributes, flags) == 0, "unexpected flags offset");
static_assert(offsetof(roxy_terminal_attributes, interrupt_byte) == 4,
              "unexpected interrupt_byte offset");
static_assert(offsetof(roxy_terminal_attributes, erase_byte) == 5, "unexpected erase_byte offset");
static_assert(offsetof(roxy_terminal_attributes, reserved) == 6, "unexpected reserved offset");

int terminal_ioctl(int fd, unsigned long request, void *argument) {
	int output;

	return sysdep<Ioctl>(fd, request, argument, &output);
}

roxy_terminal_attributes termios_to_roxy_term_attr(const struct termios *attributes) {
	/* TODO(missing-capability: unmapped POSIX terminal attributes): software flow control
	 * (IXON/IXOFF), parity and modem control, line speeds, and every control character other
	 * than VINTR/VERASE have no Roxy field, so they are dropped here rather than reaching the
	 * kernel. Only the attributes below reach it. */
	roxy_terminal_attributes record = {};
	if(attributes->c_iflag & ICRNL)
		record.flags |= ROXY_TERM_ICRNL;
	if(attributes->c_iflag & INLCR)
		record.flags |= ROXY_TERM_INLCR;
	if(attributes->c_iflag & IGNCR)
		record.flags |= ROXY_TERM_IGNCR;
	if(attributes->c_oflag & OPOST)
		record.flags |= ROXY_TERM_OPOST;
	if(attributes->c_oflag & ONLCR)
		record.flags |= ROXY_TERM_ONLCR;
	if(attributes->c_lflag & ISIG)
		record.flags |= ROXY_TERM_ISIG;
	if(attributes->c_lflag & ICANON)
		record.flags |= ROXY_TERM_ICANON;
	if(attributes->c_lflag & ECHO)
		record.flags |= ROXY_TERM_ECHO;
	record.interrupt_byte = attributes->c_cc[VINTR];
	record.erase_byte = attributes->c_cc[VERASE];
	return record;
}

void roxy_term_attr_to_termios(const roxy_terminal_attributes &record, struct termios *attributes) {
	/* Reports the values the kernel synthesizes for the fields termios_to_roxy_term_attr drops,
	 * not what a caller last set: those fields have no kernel state to read back. */
	*attributes = {};
	if(record.flags & ROXY_TERM_ICRNL)
		attributes->c_iflag |= ICRNL;
	if(record.flags & ROXY_TERM_INLCR)
		attributes->c_iflag |= INLCR;
	if(record.flags & ROXY_TERM_IGNCR)
		attributes->c_iflag |= IGNCR;
	if(record.flags & ROXY_TERM_OPOST)
		attributes->c_oflag |= OPOST;
	if(record.flags & ROXY_TERM_ONLCR)
		attributes->c_oflag |= ONLCR;
	if(record.flags & ROXY_TERM_ISIG)
		attributes->c_lflag |= ISIG;
	if(record.flags & ROXY_TERM_ICANON)
		attributes->c_lflag |= ICANON;
	if(record.flags & ROXY_TERM_ECHO)
		attributes->c_lflag |= ECHO;
	attributes->c_cflag = CS8;
	attributes->c_cc[VINTR] = record.interrupt_byte;
	attributes->c_cc[VERASE] = record.erase_byte;
	attributes->c_cc[VMIN] = 1;
}

} // namespace

int Sysdeps<Ioctl>::operator()(int fd, unsigned long request, void *argument, int *output) {
	auto result = roxy_syscall3(
	    ROXY_SYS_IOCTL,
	    fd,
	    static_cast<long>(request),
	    reinterpret_cast<long>(argument)
	);
	if(result.error)
		return static_cast<int>(result.error);

	if(output)
		*output = static_cast<int>(result.value);
	return 0;
}

int Sysdeps<Tcgetattr>::operator()(int fd, struct termios *attributes) {
	roxy_terminal_attributes record = {};

	if(int error = terminal_ioctl(fd, TCGETS, &record); error)
		return error;

	roxy_term_attr_to_termios(record, attributes);
	return 0;
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

	roxy_terminal_attributes record = termios_to_roxy_term_attr(attributes);
	return terminal_ioctl(fd, request, &record);
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

} // namespace mlibc
