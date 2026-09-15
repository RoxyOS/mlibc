#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include <stddef.h>
#include <stdint.h>

#include "errors.hpp"

/*
 * Roxy's `waitpid` status record.
 *
 * `kind` names the state change the kernel observed, and `code` carries the exit code for
 * `EXITED`, the signal number for `SIGNALED` and `STOPPED`, and zero for `CONTINUED`. The kernel
 * produces the record and this libc renders it as the POSIX wait-status word `WIFEXITED` and its
 * neighbours decode, so the POSIX bit layout stops at this boundary.
 *
 * The kind word needs no base above another personality's numbering the way a userspace-supplied
 * word does, because the kernel produces it and only this libc reads it; zero is reserved instead,
 * so an all-zero record is never a status.
 *
 * A record lives here rather than in `roxy/syscall.h` because this file is its only reader and
 * writer: the syscall numbering is one namespace every sysdep issues from, but a record describes
 * one syscall's own layout. The kernel side is `kernel/syscall/src/syscalls/waitpid.rs`; this is
 * the userspace half of the same hand-maintained contract.
 */
#define ROXY_WAIT_KIND_EXITED 1
#define ROXY_WAIT_KIND_SIGNALED 2
#define ROXY_WAIT_KIND_STOPPED 3
#define ROXY_WAIT_KIND_CONTINUED 4

typedef struct {
	uint32_t kind;
	uint32_t code;
} roxy_wait_status;

static_assert(sizeof(roxy_wait_status) == 8);
static_assert(alignof(roxy_wait_status) == 4);
static_assert(offsetof(roxy_wait_status, kind) == 0);
static_assert(offsetof(roxy_wait_status, code) == 4);

namespace mlibc {

namespace {

// Renders a Roxy wait record as the POSIX wait-status word that <sys/wait.h>'s `W` macros decode.
// The kernel reports the state change itself, so encoding that word is the libc's job and the
// POSIX bit layout stops here.
int encode_wait_status(const roxy_wait_status &record) {
	switch(record.kind) {
	case ROXY_WAIT_KIND_EXITED: return static_cast<int>((record.code & 0xff) << 8);
	case ROXY_WAIT_KIND_SIGNALED: return static_cast<int>(record.code & 0x7f);
	case ROXY_WAIT_KIND_STOPPED: return static_cast<int>(0x7f | ((record.code & 0xff) << 8));
	case ROXY_WAIT_KIND_CONTINUED: return 0xffff;
	}

	// The kernel writes only the kinds above, so no other value reaches this encoder.
	return 0;
}

} // namespace

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
	roxy_wait_status record = {};
	auto result = roxy_syscall4(
	    ROXY_SYS_WAITPID,
	    pid,
	    reinterpret_cast<long>(&record),
	    flags,
	    reinterpret_cast<long>(ru)
	);
	if(result.error)
		return static_cast<int>(result.error);

	*ret_pid = static_cast<pid_t>(result.value);

	// A zero return is a non-blocking wait that observed no state change, so the kernel wrote no
	// record and the caller must not read one.
	if(status && result.value > 0)
		*status = encode_wait_status(record);
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
