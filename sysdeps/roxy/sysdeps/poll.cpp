#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include "errors.hpp"

namespace mlibc {

int Sysdeps<Poll>::operator()(struct pollfd *fds, nfds_t count, int timeout, int *num_events) {
	auto result = roxy_syscall3(
	    ROXY_SYS_POLL,
	    reinterpret_cast<long>(fds),
	    count,
	    timeout
	);
	if(result.error)
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
	auto result = roxy_syscall4(
	    ROXY_SYS_PPOLL,
	    reinterpret_cast<long>(fds),
	    count,
	    reinterpret_cast<long>(timeout),
	    reinterpret_cast<long>(signal_mask)
	);
	if(result.error)
		return static_cast<int>(result.error);

	*num_events = static_cast<int>(result.value);
	return 0;
}

int Sysdeps<Pselect>::operator()(
	int num_fds,
	fd_set *read_set,
	fd_set *write_set,
	fd_set *except_set,
	const struct timespec *timeout,
	const sigset_t *signal_mask,
	int *num_events
) {
	auto result = roxy_syscall6(
	    ROXY_SYS_PSELECT,
	    num_fds,
	    reinterpret_cast<long>(read_set),
	    reinterpret_cast<long>(write_set),
	    reinterpret_cast<long>(except_set),
	    reinterpret_cast<long>(timeout),
	    reinterpret_cast<long>(signal_mask)
	);
	if(result.error)
		return static_cast<int>(result.error);

	*num_events = static_cast<int>(result.value);
	return 0;
}

} // namespace mlibc
