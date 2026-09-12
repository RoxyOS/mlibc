#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include <stdint.h>

#include "errors.hpp"

namespace mlibc {

pid_t Sysdeps<GetPid>::operator()() {
	// The kernel cannot fail this.
	return static_cast<pid_t>(roxy_syscall0(ROXY_SYS_GETPID).value);
}

pid_t Sysdeps<GetPpid>::operator()() {
	// The kernel cannot fail this.
	return static_cast<pid_t>(roxy_syscall0(ROXY_SYS_GETPPID).value);
}

int Sysdeps<Fork>::operator()(pid_t *child) {
	auto result = roxy_syscall1(ROXY_SYS_FORK, 0);
	if(result.error)
		return static_cast<int>(result.error);
	if(result.value > INT32_MAX)
		return EOVERFLOW;

	*child = static_cast<pid_t>(result.value);
	return 0;
}

int Sysdeps<Waitpid>::operator()(
	pid_t pid,
	int *status,
	int flags,
	struct rusage *ru,
	pid_t *ret_pid
) {
	auto result = roxy_syscall4(
	    ROXY_SYS_WAITPID,
	    pid,
	    reinterpret_cast<long>(status),
	    flags,
	    reinterpret_cast<long>(ru)
	);
	if(result.error)
		return static_cast<int>(result.error);

	*ret_pid = static_cast<pid_t>(result.value);
	return 0;
}

int Sysdeps<Execve>::operator()(const char *path, char *const argv[], char *const envp[]) {
	return syscall_error(roxy_syscall3(
	    ROXY_SYS_EXECVE,
	    reinterpret_cast<long>(path),
	    reinterpret_cast<long>(argv),
	    reinterpret_cast<long>(envp)
	));
}

void Sysdeps<Exit>::operator()(int status) {
	roxy_syscall1(ROXY_SYS_EXIT, status);
	__builtin_unreachable();
}

int Sysdeps<SetPgid>::operator()(pid_t pid, pid_t pgid) {
	return syscall_error(roxy_syscall2(ROXY_SYS_SET_PGID, pid, pgid));
}

int Sysdeps<GetPgid>::operator()(pid_t pid, pid_t *pgid) {
	auto result = roxy_syscall1(ROXY_SYS_GET_PGID, pid);
	if(result.error)
		return static_cast<int>(result.error);

	*pgid = static_cast<pid_t>(result.value);
	return 0;
}

int Sysdeps<SetSid>::operator()(pid_t *sid) {
	auto result = roxy_syscall0(ROXY_SYS_SET_SID);
	if(result.error)
		return static_cast<int>(result.error);

	*sid = static_cast<pid_t>(result.value);
	return 0;
}

} // namespace mlibc
