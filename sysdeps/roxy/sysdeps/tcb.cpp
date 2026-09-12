#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include "errors.hpp"

namespace mlibc {

// Points the calling thread's local-storage base at `pointer`, the TCB whose layout mlibc's TLS
// starts with. Kept out of thread.cpp because the dynamic loader sets up its own TCB before any
// thread machinery exists, and thread.cpp carries pthread code the loader must not link.
int Sysdeps<TcbSet>::operator()(void *pointer) {
	return syscall_error(roxy_syscall1(ROXY_SYS_TCB_SET, reinterpret_cast<long>(pointer)));
}

} // namespace mlibc
