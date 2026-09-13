#ifndef _ABIBITS_VM_FLAGS_H
#define _ABIBITS_VM_FLAGS_H

#include <mlibc-config.h>

/*
 * Roxy's memory protection bits and `mmap` flags.
 *
 * Each word is numbered from its own base, above Linux's range for that word, so a value below a
 * base is another personality's numbering and the kernel reports it as foreign instead of reading
 * it as a request of its own. Each bit is its own flag rather than a shared one, so a caller's `&`
 * test for one bit cannot answer true because another was requested.
 *
 * `PROT_NONE` and `MAP_FILE` stay zero: they are the absence of a request, not a numbered one.
 * Linux's `MAP_GROWSDOWN`, `MAP_DENYWRITE`, `MAP_EXECUTABLE`, `MAP_LOCKED`, `MAP_NORESERVE`,
 * `MAP_POPULATE`, `MAP_NONBLOCK`, `MAP_STACK`, `MAP_HUGETLB`, `MAP_SYNC`, and
 * `MAP_FIXED_NOREPLACE` are absent: Roxy implements none of them.
 *
 * The kernel side is `kernel/syscall/src/syscalls/vm/mod.rs` (protection) and
 * `kernel/syscall/src/syscalls/vm/map.rs` (flags).
 */
#define ROXY_PROTECTION_BASE (1 << 20)

#define PROT_NONE  0x00
#define PROT_READ  ROXY_PROTECTION_BASE
#define PROT_WRITE (ROXY_PROTECTION_BASE << 1)
#define PROT_EXEC  (ROXY_PROTECTION_BASE << 2)

#define ROXY_MAP_FLAGS_BASE (1 << 21)

#define MAP_FAILED ((void *)(-1))
#define MAP_FILE    0x00
#define MAP_SHARED  ROXY_MAP_FLAGS_BASE
#define MAP_PRIVATE (ROXY_MAP_FLAGS_BASE << 1)
#define MAP_FIXED   (ROXY_MAP_FLAGS_BASE << 2)
#define MAP_ANON    (ROXY_MAP_FLAGS_BASE << 3)
#define MAP_ANONYMOUS (ROXY_MAP_FLAGS_BASE << 3)

#define MS_ASYNC 0x01
#define MS_INVALIDATE 0x02
#define MS_SYNC 0x04

#define MCL_CURRENT 0x01
#define MCL_FUTURE 0x02

#define POSIX_MADV_NORMAL 0
#define POSIX_MADV_RANDOM 1
#define POSIX_MADV_SEQUENTIAL 2
#define POSIX_MADV_WILLNEED 3
#define POSIX_MADV_DONTNEED 4

#if __MLIBC_LINUX_OPTION

#if defined(_DEFAULT_SOURCE)
#define MADV_NORMAL 0
#define MADV_RANDOM 1
#define MADV_SEQUENTIAL 2
#define MADV_WILLNEED 3
#define MADV_DONTNEED 4
#define MADV_FREE 8
#define MADV_REMOVE 9
#define MADV_DONTFORK 10
#define MADV_DOFORK 11
#define MADV_MERGEABLE 12
#define MADV_UNMERGEABLE 13
#define MADV_HUGEPAGE 14
#define MADV_NOHUGEPAGE 15
#define MADV_DONTDUMP 16
#define MADV_DODUMP 17
#define MADV_WIPEONFORK 18
#define MADV_KEEPONFORK 19
#define MADV_COLD 20
#define MADV_PAGEOUT 21
#define MADV_HWPOISON 100
#define MADV_SOFT_OFFLINE 101
#endif /* defined(_DEFAULT_SOURCE) */

#if defined(_GNU_SOURCE)
#define MREMAP_MAYMOVE 1
#define MREMAP_FIXED 2

#define MFD_CLOEXEC 1U
#define MFD_ALLOW_SEALING 2U
#define MFD_HUGETLB 4U
#endif /* defined(_GNU_SOURCE) */

#endif /* __MLIBC_LINUX_OPTION */

#endif /* _ABIBITS_VM_FLAGS_H */
