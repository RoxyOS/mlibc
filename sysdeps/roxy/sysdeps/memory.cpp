#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include <stddef.h>

#include "errors.hpp"

namespace mlibc {

int Sysdeps<AnonAllocate>::operator()(size_t size, void **pointer) {
	auto result = roxy_syscall1(ROXY_SYS_ANON_ALLOCATE, size);
	if(result < 0)
		return static_cast<int>(-result);

	*pointer = reinterpret_cast<void *>(result);
	return 0;
}

int Sysdeps<AnonFree>::operator()(void *pointer, size_t size) {
	return syscall_error(
	    roxy_syscall2(ROXY_SYS_ANON_FREE, reinterpret_cast<long>(pointer), size)
	);
}

int Sysdeps<VmMap>::operator()(
	void *hint,
	size_t size,
	int prot,
	int flags,
	int fd,
	off_t offset,
	void **window
) {
	auto result = roxy_syscall6(
	    ROXY_SYS_VM_MAP,
	    reinterpret_cast<long>(hint),
	    size,
	    prot,
	    flags,
	    fd,
	    offset
	);
	if(result < 0)
		return static_cast<int>(-result);

	*window = reinterpret_cast<void *>(result);
	return 0;
}

int Sysdeps<VmUnmap>::operator()(void *pointer, size_t size) {
	return syscall_error(
	    roxy_syscall2(ROXY_SYS_VM_UNMAP, reinterpret_cast<long>(pointer), size)
	);
}

int Sysdeps<VmProtect>::operator()(void *pointer, size_t size, int protection) {
	return syscall_error(roxy_syscall3(
	    ROXY_SYS_VM_PROTECT,
	    reinterpret_cast<long>(pointer),
	    size,
	    protection
	));
}

} // namespace mlibc
