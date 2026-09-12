#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

namespace mlibc {

gid_t Sysdeps<GetGid>::operator()() {
	// The kernel cannot fail this; every ID is 0 until a user model exists.
	return static_cast<gid_t>(roxy_syscall0(ROXY_SYS_GETGID).value);
}

gid_t Sysdeps<GetEgid>::operator()() {
	// The kernel cannot fail this; every ID is 0 until a user model exists.
	return static_cast<gid_t>(roxy_syscall0(ROXY_SYS_GETEGID).value);
}

uid_t Sysdeps<GetUid>::operator()() {
	// The kernel cannot fail this; every ID is 0 until a user model exists.
	return static_cast<uid_t>(roxy_syscall0(ROXY_SYS_GETUID).value);
}

uid_t Sysdeps<GetEuid>::operator()() {
	// The kernel cannot fail this; every ID is 0 until a user model exists.
	return static_cast<uid_t>(roxy_syscall0(ROXY_SYS_GETEUID).value);
}

int Sysdeps<GetResuid>::operator()(uid_t *ruid, uid_t *euid, uid_t *suid) {
	// Roxy has no user model yet; every ID is 0 (root).
	*ruid = 0;
	*euid = 0;
	*suid = 0;
	return 0;
}

int Sysdeps<GetResgid>::operator()(gid_t *rgid, gid_t *egid, gid_t *sgid) {
	// Roxy has no user model yet; every ID is 0 (root).
	*rgid = 0;
	*egid = 0;
	*sgid = 0;
	return 0;
}

int Sysdeps<SetUid>::operator()(uid_t uid) {
	// Roxy currently runs every process as root and has no credential state.
	return uid == 0 ? 0 : EPERM;
}

int Sysdeps<SetGid>::operator()(gid_t gid) {
	// Roxy currently runs every process as root and has no credential state.
	return gid == 0 ? 0 : EPERM;
}

int Sysdeps<SetEuid>::operator()(uid_t euid) {
	// Roxy has no credential state; effective uid is always root.
	return euid == 0 ? 0 : EPERM;
}

int Sysdeps<SetEgid>::operator()(gid_t egid) {
	// Roxy has no credential state; effective gid is always root.
	return egid == 0 ? 0 : EPERM;
}

} // namespace mlibc
