#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>

#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include "errors.hpp"

/*
 * Roxy's `poll` request record and its condition words.
 *
 * One record per descriptor. `requested_events` names the conditions the caller waits for and
 * `reported_events` the conditions the kernel observed to hold; both words are Roxy's own, one bit
 * per condition. Because `requested_events` is a field of this record rather than a word shared
 * with another personality, it has no foreign numbering to tell apart from Roxy's, and the kernel
 * reports a bit no condition names as undefined. A negative `fd` reports nothing for its record.
 *
 * `ERROR` and `HANGUP` are reported whatever the request named, so no request can name them, while
 * `INVALID_DESCRIPTOR` stands in for a descriptor that is not open.
 *
 * A record lives here rather than in `roxy/syscall.h` because this file is its only reader and
 * writer: the syscall numbering is one namespace every sysdep issues from, but a record describes
 * one syscall's own layout. The kernel side is `kernel/syscall/src/syscalls/poll/abi.rs`; this is
 * the userspace half of the same hand-maintained contract.
 */
#define ROXY_POLL_READABLE (1u << 0)
#define ROXY_POLL_PRIORITY (1u << 1)
#define ROXY_POLL_WRITABLE (1u << 2)
#define ROXY_POLL_ERROR (1u << 3)
#define ROXY_POLL_HANGUP (1u << 4)
#define ROXY_POLL_INVALID_DESCRIPTOR (1u << 5)

typedef struct {
	int32_t fd;
	uint32_t requested_events;
	uint32_t reported_events;
} roxy_poll_request;

static_assert(sizeof(roxy_poll_request) == 12);
static_assert(alignof(roxy_poll_request) == 4);
static_assert(offsetof(roxy_poll_request, fd) == 0);
static_assert(offsetof(roxy_poll_request, requested_events) == 4);
static_assert(offsetof(roxy_poll_request, reported_events) == 8);

namespace mlibc {

namespace {

// The conditions a caller can wait for. `POLLERR`, `POLLHUP`, and `POLLNVAL` are report-only, so
// naming one in `events` asks for nothing.
constexpr short requestable_events = POLLIN | POLLPRI | POLLOUT;

// Translates the conditions a caller waits for into the kernel's request word. A condition outside
// the six POSIX names is one Roxy does not serve; the Roxy headers name none of them, so only a
// hardcoded value reaches this test, and reporting it answers the caller instead of letting it
// wait forever for a condition that can never be reported.
bool asks_for_unserved_condition(short events) {
	return (events & ~(requestable_events | POLLERR | POLLHUP | POLLNVAL)) != 0;
}

// Translates the POSIX `pollfd.events` word into Roxy's `requested_events` word.
uint32_t to_roxy_requested_events(short posix_events) {
	uint32_t requested = 0;

	if (posix_events & POLLIN)
		requested |= ROXY_POLL_READABLE;
	if (posix_events & POLLPRI)
		requested |= ROXY_POLL_PRIORITY;
	if (posix_events & POLLOUT)
		requested |= ROXY_POLL_WRITABLE;

	return requested;
}

// Translates Roxy's `reported_events` word into the POSIX `pollfd.revents` word.
short to_posix_revents(uint32_t reported_events) {
	short revents = 0;

	if (reported_events & ROXY_POLL_READABLE)
		revents |= POLLIN;
	if (reported_events & ROXY_POLL_PRIORITY)
		revents |= POLLPRI;
	if (reported_events & ROXY_POLL_WRITABLE)
		revents |= POLLOUT;
	if (reported_events & ROXY_POLL_ERROR)
		revents |= POLLERR;
	if (reported_events & ROXY_POLL_HANGUP)
		revents |= POLLHUP;
	if (reported_events & ROXY_POLL_INVALID_DESCRIPTOR)
		revents |= POLLNVAL;

	return revents;
}

// Copies the caller's descriptors into the kernel's request records.
//
// The two layouts cannot be shared: `struct pollfd` is upstream mlibc's `short`-wide pair, so it
// has no room for the words Roxy's record carries. A negative descriptor reports nothing whatever
// its `events` names, so its conditions are ignored rather than translated.
//
// Returns `nullptr` with `*error` set when the array cannot be translated, and `nullptr` with a
// zero error for an empty request, which the kernel serves as a wait without descriptors.
roxy_poll_request *to_roxy_requests(struct pollfd *fds, nfds_t count, int *error) {
	*error = 0;

	if (count == 0)
		return nullptr;

	// `count` is the caller's `nfds_t`, so the byte length it implies must be checked rather than
	// wrapped: a count that overflows `size_t` would size the allocation smaller than the array the
	// loop below fills. The kernel rejects the same count as invalid.
	if (count > SIZE_MAX / sizeof(roxy_poll_request)) {
		*error = EINVAL;
		return nullptr;
	}

	auto *requests = static_cast<roxy_poll_request *>(malloc(count * sizeof(roxy_poll_request)));
	if (!requests) {
		*error = ENOMEM;
		return nullptr;
	}

	for (nfds_t index = 0; index < count; index++) {
		if (fds[index].fd >= 0 && asks_for_unserved_condition(fds[index].events)) {
			free(requests);
			*error = EINVAL;
			return nullptr;
		}

		requests[index].fd = fds[index].fd;
		requests[index].requested_events =
		    (fds[index].fd < 0) ? 0 : to_roxy_requested_events(fds[index].events);
		requests[index].reported_events = 0;
	}

	return requests;
}

void write_posix_revents(roxy_poll_request *requests, struct pollfd *fds, nfds_t count) {
	for (nfds_t index = 0; index < count; index++)
		fds[index].revents = to_posix_revents(requests[index].reported_events);
}

} // namespace

int Sysdeps<Poll>::operator()(struct pollfd *fds, nfds_t count, int timeout, int *num_events) {
	int error = 0;
	roxy_poll_request *requests = to_roxy_requests(fds, count, &error);
	if (error)
		return error;

	auto result = roxy_syscall3(
	    ROXY_SYS_POLL,
	    reinterpret_cast<long>(requests),
	    count,
	    timeout
	);

	if (!result.error)
		write_posix_revents(requests, fds, count);

	free(requests);

	if (result.error)
		return static_cast<int>(result.error);

	*num_events = static_cast<int>(result.value);
	return 0;
}

int Sysdeps<Ppoll>::operator()(
	struct pollfd *fds,
	nfds_t count,
	const struct timespec *timeout,
	const sigset_t *signal_mask,
	int *num_events
) {
	int error = 0;
	roxy_poll_request *requests = to_roxy_requests(fds, count, &error);
	if (error)
		return error;

	auto result = roxy_syscall4(
	    ROXY_SYS_PPOLL,
	    reinterpret_cast<long>(requests),
	    count,
	    reinterpret_cast<long>(timeout),
	    reinterpret_cast<long>(signal_mask)
	);

	if (!result.error)
		write_posix_revents(requests, fds, count);

	free(requests);

	if (result.error)
		return static_cast<int>(result.error);

	*num_events = static_cast<int>(result.value);
	return 0;
}

// Renders `select` and `pselect` on the kernel's `ppoll`, which is the same wait with the same
// atomic signal-mask replacement. The kernel serves no descriptor-set layout of its own: a set is
// this library's type, so the translation belongs here.
int Sysdeps<Pselect>::operator()(
	int num_fds,
	fd_set *read_set,
	fd_set *write_set,
	fd_set *except_set,
	const struct timespec *timeout,
	const sigset_t *signal_mask,
	int *num_events
) {
	if (num_fds < 0 || num_fds > FD_SETSIZE)
		return EINVAL;

	roxy_poll_request *requests = nullptr;
	if (num_fds > 0) {
		requests = static_cast<roxy_poll_request *>(malloc(
		    static_cast<size_t>(num_fds) * sizeof(roxy_poll_request)
		));
		if (!requests)
			return ENOMEM;
	}

	// One record per descriptor any set names, in ascending descriptor order. A descriptor no set
	// names is not waited on, and an empty request is the wait a descriptor-less `select` asks for.
	int count = 0;
	for (int fd = 0; fd < num_fds; fd++) {
		uint32_t requested = 0;

		if (read_set && __FD_ISSET(fd, read_set))
			requested |= ROXY_POLL_READABLE;
		if (write_set && __FD_ISSET(fd, write_set))
			requested |= ROXY_POLL_WRITABLE;
		if (except_set && __FD_ISSET(fd, except_set))
			requested |= ROXY_POLL_PRIORITY;

		if (!requested)
			continue;

		requests[count].fd = fd;
		requests[count].requested_events = requested;
		requests[count].reported_events = 0;
		count++;
	}

	auto result = roxy_syscall4(
	    ROXY_SYS_PPOLL,
	    reinterpret_cast<long>(requests),
	    static_cast<long>(count),
	    reinterpret_cast<long>(timeout),
	    reinterpret_cast<long>(signal_mask)
	);

	if (result.error) {
		free(requests);
		return static_cast<int>(result.error);
	}

	for (int index = 0; index < count; index++) {
		if (requests[index].reported_events & ROXY_POLL_INVALID_DESCRIPTOR) {
			free(requests);
			// POSIX reports a set naming a descriptor that is not open as EBADF and leaves the
			// sets as the caller passed them, so nothing is written back here.
			return EBADF;
		}
	}

	if (read_set)
		__FD_ZERO(read_set);
	if (write_set)
		__FD_ZERO(write_set);
	if (except_set)
		__FD_ZERO(except_set);

	// A reported condition was requested by the set that named its descriptor, so the set for a
	// reported bit is never the null one. The count is this library's own rather than the kernel's:
	// `select` returns the bits it set, and a descriptor reporting only a condition no set asked
	// for would set none.
	int ready = 0;
	for (int index = 0; index < count; index++) {
		int fd = requests[index].fd;
		uint32_t reported = requests[index].reported_events;

		if (reported & ROXY_POLL_READABLE) {
			__FD_SET(fd, read_set);
			ready++;
		}

		if (reported & ROXY_POLL_WRITABLE) {
			__FD_SET(fd, write_set);
			ready++;
		}

		if (reported & ROXY_POLL_PRIORITY) {
			__FD_SET(fd, except_set);
			ready++;
		}
	}

	free(requests);

	*num_events = ready;
	return 0;
}

} // namespace mlibc
