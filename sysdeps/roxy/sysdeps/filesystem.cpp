#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <mlibc/all-sysdeps.hpp>
#include <roxy/syscall.h>

#include <sys/stat.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The records these sysdeps carry.
 *
 * A record lives here rather than in `roxy/syscall.h` because this file is its only reader and
 * writer: the syscall numbering is one namespace every sysdep issues from, but a record describes
 * one syscall's own layout. The kernel side is `kernel/syscall/src/syscalls/fs/stat.rs` for the
 * `stat` result and `kernel/syscall/src/syscalls/fs/read_entries.rs` for the directory entry; each
 * is the userspace half of the same hand-maintained contract.
 */

/* The kind of file a `stat` result or a directory entry describes.
 *
 * The values are Roxy's own rather than POSIX's `S_IFMT` and `d_type` numbering, which are rendered
 * from these at the two places userspace compares them: `posix_mode_of` and `dirent_type_of` below.
 * Zero is reserved, so an all-zero word names no kind. The kernel side is
 * `kernel/syscall/src/syscalls/fs/mod.rs`.
 */
#define ROXY_FILE_KIND_REGULAR 1
#define ROXY_FILE_KIND_DIRECTORY 2
#define ROXY_FILE_KIND_SYMLINK 3
#define ROXY_FILE_KIND_BLOCK_DEVICE 4
#define ROXY_FILE_KIND_CHARACTER_DEVICE 5
#define ROXY_FILE_KIND_FIFO 6
#define ROXY_FILE_KIND_SOCKET 7
#define ROXY_FILE_KIND_UNKNOWN 8

/* The `stat` result, as `Stat` writes it.
 *
 * `kind` and `permissions` are separate fields, so a caller testing one cannot match bits of the
 * other. `permissions` keeps the POSIX `rwxrwxrwx`-plus-special-bit numbering, which is what
 * `chmod`, `mkdir`, and `open` pass and the filesystem stores; `posix_mode_of` combines it with the
 * type bits `st_mode` carries. `reserved` is always zero and makes the record's tail padding
 * explicit. */
typedef struct {
	uint64_t file_id;
	uint64_t size;
	uint64_t blocks;
	uint64_t hard_links;
	uint32_t kind;
	uint32_t permissions;
	uint32_t block_size;
	uint32_t reserved;
} roxy_stat_result;

static_assert(sizeof(roxy_stat_result) == 48);
static_assert(alignof(roxy_stat_result) == 8);
static_assert(offsetof(roxy_stat_result, file_id) == 0);
static_assert(offsetof(roxy_stat_result, size) == 8);
static_assert(offsetof(roxy_stat_result, blocks) == 16);
static_assert(offsetof(roxy_stat_result, hard_links) == 24);
static_assert(offsetof(roxy_stat_result, kind) == 32);
static_assert(offsetof(roxy_stat_result, permissions) == 36);
static_assert(offsetof(roxy_stat_result, block_size) == 40);
static_assert(offsetof(roxy_stat_result, reserved) == 44);

/* One directory entry. The kernel serializes these into the caller's buffer and `ReadEntries` hands
 * that buffer back as an array of `struct dirent`, so the two layouts have to coincide field for
 * field; the assertions below pin that, which is what makes the record's own offsets the POSIX
 * ones and needs no second copy of them here. The `type` byte carries a `ROXY_FILE_KIND_*` word
 * rather than a `DT_*` one, and `ReadEntries` renders it in place before returning. */
typedef struct {
	uint64_t inode;
	int64_t offset;
	uint16_t record_size;
	uint8_t type;
	char name[256];
	uint8_t padding[5];
} roxy_dirent;

static_assert(sizeof(roxy_dirent) == sizeof(struct dirent));
static_assert(offsetof(roxy_dirent, inode) == offsetof(struct dirent, d_ino));
static_assert(offsetof(roxy_dirent, offset) == offsetof(struct dirent, d_off));
static_assert(offsetof(roxy_dirent, record_size) == offsetof(struct dirent, d_reclen));
static_assert(offsetof(roxy_dirent, type) == offsetof(struct dirent, d_type));
static_assert(offsetof(roxy_dirent, name) == offsetof(struct dirent, d_name));

namespace {

/* The `S_IF*` type bits a `st_mode` carries for a Roxy file kind.
 *
 * POSIX numbers those bits itself and `S_ISDIR` and its neighbours read them, so the translation
 * from Roxy's kind word belongs here, where the record is decoded. A kind the kernel reports as
 * unknown contributes no type bit, which is how POSIX spells a `st_mode` whose type is not known. */
mode_t file_type_bits(uint32_t kind) {
	switch(kind) {
	case ROXY_FILE_KIND_REGULAR: return S_IFREG;
	case ROXY_FILE_KIND_DIRECTORY: return S_IFDIR;
	case ROXY_FILE_KIND_SYMLINK: return S_IFLNK;
	case ROXY_FILE_KIND_BLOCK_DEVICE: return S_IFBLK;
	case ROXY_FILE_KIND_CHARACTER_DEVICE: return S_IFCHR;
	case ROXY_FILE_KIND_FIFO: return S_IFIFO;
	case ROXY_FILE_KIND_SOCKET: return S_IFSOCK;
	default: return 0;
	}
}

/* The `st_mode` a `stat` result describes: the kind as POSIX type bits plus the stored permission
 * bits, which are already the ones `st_mode` holds. */
mode_t posix_mode_of(uint32_t kind, uint32_t permissions) {
	return file_type_bits(kind) | static_cast<mode_t>(permissions);
}

/* The `DT_*` byte a directory entry reports for a Roxy file kind. */
unsigned char dirent_type_of(uint32_t kind) {
	switch(kind) {
	case ROXY_FILE_KIND_REGULAR: return DT_REG;
	case ROXY_FILE_KIND_DIRECTORY: return DT_DIR;
	case ROXY_FILE_KIND_SYMLINK: return DT_LNK;
	case ROXY_FILE_KIND_BLOCK_DEVICE: return DT_BLK;
	case ROXY_FILE_KIND_CHARACTER_DEVICE: return DT_CHR;
	case ROXY_FILE_KIND_FIFO: return DT_FIFO;
	case ROXY_FILE_KIND_SOCKET: return DT_SOCK;
	default: return DT_UNKNOWN;
	}
}

/* Rewrites each record's kind word as the `DT_*` byte userspace compares against.
 *
 * The kernel serializes `roxy_dirent` records into the caller's buffer and `ReadEntries` hands that
 * buffer back as an array of `struct dirent`, so a record's kind byte is an entry's `d_type` and
 * can be rendered in place, without a second buffer. The walk takes each record's own `d_reclen`,
 * which the kernel writes as the record size, so it assumes nothing about how many records a call
 * returned. */
void render_dirent_types(void *buffer, size_t bytes_read) {
	auto *bytes = static_cast<unsigned char *>(buffer);
	size_t offset = 0;

	while(offset + offsetof(struct dirent, d_name) < bytes_read) {
		auto *entry = reinterpret_cast<struct dirent *>(bytes + offset);

		if(!entry->d_reclen)
			break;

		entry->d_type = dirent_type_of(entry->d_type);
		offset += entry->d_reclen;
	}
}

} // namespace

namespace mlibc {

int Sysdeps<Chdir>::operator()(const char *path) {
	auto result = roxy_syscall1(ROXY_SYS_CHDIR, reinterpret_cast<long>(path));

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<GetCwd>::operator()(char *buffer, size_t size) {
	auto result = roxy_syscall2(
	    ROXY_SYS_GETCWD,
	    reinterpret_cast<long>(buffer),
	    size
	);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<OpenDir>::operator()(const char *path, int *handle) {
	auto result = roxy_syscall1(ROXY_SYS_OPEN_DIR, reinterpret_cast<long>(path));
	if(result.error)
		return static_cast<int>(result.error);

	*handle = static_cast<int>(result.value);
	return 0;
}

int Sysdeps<ReadEntries>::operator()(
	int handle,
	void *buffer,
	size_t max_size,
	size_t *bytes_read
) {
	auto result = roxy_syscall3(
	    ROXY_SYS_READ_ENTRIES,
	    handle,
	    reinterpret_cast<long>(buffer),
	    max_size
	);
	if(result.error)
		return static_cast<int>(result.error);

	*bytes_read = static_cast<size_t>(result.value);
	render_dirent_types(buffer, *bytes_read);
	return 0;
}

int Sysdeps<Mkdir>::operator()(const char *path, mode_t mode) {
	return sysdep<Mkdirat>(AT_FDCWD, path, mode);
}

int Sysdeps<Mkdirat>::operator()(int dirfd, const char *path, mode_t mode) {
	auto result = roxy_syscall3(
	    ROXY_SYS_MKDIRAT,
	    dirfd,
	    reinterpret_cast<long>(path),
	    mode
	);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Rmdir>::operator()(const char *path) {
	return sysdep<Unlinkat>(AT_FDCWD, path, AT_REMOVEDIR);
}

int Sysdeps<Unlinkat>::operator()(int dirfd, const char *path, int flags) {
	auto result = roxy_syscall3(
	    ROXY_SYS_UNLINKAT,
	    dirfd,
	    reinterpret_cast<long>(path),
	    flags
	);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Readlink>::operator()(
	const char *path,
	void *buffer,
	size_t max_size,
	ssize_t *length
) {
	return sysdep<Readlinkat>(AT_FDCWD, path, buffer, max_size, length);
}

int Sysdeps<Readlinkat>::operator()(
	int dirfd,
	const char *path,
	void *buffer,
	size_t max_size,
	ssize_t *length
) {
	auto result = roxy_syscall4(
	    ROXY_SYS_READLINKAT,
	    dirfd,
	    reinterpret_cast<long>(path),
	    reinterpret_cast<long>(buffer),
	    max_size
	);
	if(result.error)
		return static_cast<int>(result.error);

	*length = static_cast<ssize_t>(result.value);
	return 0;
}

int Sysdeps<Link>::operator()(const char *old_path, const char *new_path) {
	return sysdep<Linkat>(AT_FDCWD, old_path, AT_FDCWD, new_path, 0);
}

int Sysdeps<Linkat>::operator()(
	int olddirfd,
	const char *old_path,
	int newdirfd,
	const char *new_path,
	int flags
) {
	auto result = roxy_syscall5(
	    ROXY_SYS_LINKAT,
	    olddirfd,
	    reinterpret_cast<long>(old_path),
	    newdirfd,
	    reinterpret_cast<long>(new_path),
	    flags
	);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Symlink>::operator()(const char *target_path, const char *link_path) {
	return sysdep<Symlinkat>(target_path, AT_FDCWD, link_path);
}

int Sysdeps<Symlinkat>::operator()(const char *target_path, int dirfd, const char *link_path) {
	auto result = roxy_syscall3(
	    ROXY_SYS_SYMLINKAT,
	    reinterpret_cast<long>(target_path),
	    dirfd,
	    reinterpret_cast<long>(link_path)
	);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Rename>::operator()(const char *path, const char *new_path) {
	return sysdep<Renameat>(AT_FDCWD, path, AT_FDCWD, new_path);
}

int Sysdeps<Renameat>::operator()(
	int olddirfd,
	const char *old_path,
	int newdirfd,
	const char *new_path
) {
	auto result = roxy_syscall4(
	    ROXY_SYS_RENAMEAT,
	    olddirfd,
	    reinterpret_cast<long>(old_path),
	    newdirfd,
	    reinterpret_cast<long>(new_path)
	);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

void Sysdeps<Sync>::operator()() {
	roxy_syscall0(ROXY_SYS_SYNC);
}

int Sysdeps<Fsync>::operator()(int fd) {
	auto result = roxy_syscall1(ROXY_SYS_FSYNC, fd);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Ftruncate>::operator()(int fd, size_t size) {
	auto result = roxy_syscall2(ROXY_SYS_FTRUNCATE, fd, size);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Stat>::operator()(
	fsfd_target target,
	int fd,
	const char *path,
	int flags,
	struct stat *output
) {
	roxy_stat_result stat_result;
	auto result = roxy_syscall5(
	    ROXY_SYS_STAT,
	    static_cast<long>(target),
	    fd,
	    reinterpret_cast<long>(path),
	    flags,
	    reinterpret_cast<long>(&stat_result)
	);
	if(result.error)
		return static_cast<int>(result.error);
	if(stat_result.size > INT64_MAX)
		return EOVERFLOW;

	*output = {};
	output->st_ino = stat_result.file_id;
	output->st_mode = posix_mode_of(stat_result.kind, stat_result.permissions);
	output->st_nlink = stat_result.hard_links;
	output->st_size = stat_result.size;
	output->st_blksize = stat_result.block_size;
	output->st_blocks = stat_result.blocks;
	return 0;
}

int Sysdeps<Seek>::operator()(int fd, off_t offset, int whence, off_t *new_offset) {
	auto result = roxy_syscall3(ROXY_SYS_SEEK, fd, offset, whence);
	if(result.error)
		return static_cast<int>(result.error);

	*new_offset = result.value;
	return 0;
}

int Sysdeps<Umask>::operator()(mode_t mode, mode_t *old) {
	// The kernel stores the new mask and returns the previous one.
	auto result = roxy_syscall1(ROXY_SYS_UMASK, mode);
	if(result.error)
		return static_cast<int>(result.error);

	*old = static_cast<mode_t>(result.value);
	return 0;
}

int Sysdeps<Chmod>::operator()(const char *pathname, mode_t mode) {
	auto result = roxy_syscall2(ROXY_SYS_CHMOD, reinterpret_cast<long>(pathname), mode);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Fchmod>::operator()(int fd, mode_t mode) {
	auto result = roxy_syscall2(ROXY_SYS_FCHMOD, fd, mode);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

int Sysdeps<Access>::operator()(const char *pathname, int mode) {
	auto result = roxy_syscall2(ROXY_SYS_ACCESS, reinterpret_cast<long>(pathname), mode);

	return result.value < 0 ? static_cast<int>(result.error) : 0;
}

} // namespace mlibc
