#include <mlibc/all-sysdeps.hpp>

namespace mlibc {

namespace {

constexpr int log_fd = 2;

void write_text(const char *text) {
	size_t length = 0;
	while(text[length])
		length++;

	size_t transferred = 0;
	while(transferred < length) {
		ssize_t written = 0;
		auto error = sysdep<Write>(
		    log_fd,
		    text + transferred,
		    length - transferred,
		    &written
		);
		if(error || written <= 0)
			return;

		transferred += static_cast<size_t>(written);
	}
}

} // namespace

void Sysdeps<LibcPanic>::operator()() {
	write_text("mlibc panic!\n");
	sysdep<Exit>(1);
}

void Sysdeps<LibcLog>::operator()(const char *message) {
	write_text("mlibc: ");
	write_text(message);
	write_text("\n");
}

} // namespace mlibc
