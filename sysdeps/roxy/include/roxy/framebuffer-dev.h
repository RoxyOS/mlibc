#ifndef _ROXY_FRAMEBUFFER_DEV_H
#define _ROXY_FRAMEBUFFER_DEV_H

#include <abi-bits/ioctls.h>
#include <stddef.h>
#include <stdint.h>

/* Roxy's framebuffer device protocol.
 *
 * `/dev/framebuffer` is a character device with three requests, numbered inside the framebuffer
 * block of the Roxy ioctl space (see <abi-bits/ioctls.h>). `GET_INFO` reports the layout and
 * pixel format; the pixels themselves are reached by mapping the device with `mmap(..., offset =
 * 0)`, never by read/write. `TAKE_CONTROL` and `RELEASE_CONTROL` hand the visible frame to the
 * calling process and give it back, so the kernel console stops painting over the client's pixels
 * while it owns them. The boot loader owns the mode, so there is no mode-setting request: a client
 * that wants to know whether a mode is usable compares it against what `GET_INFO` reported.
 *
 * Pixels are always 32 bits wide and always in the RGB memory model, because the kernel publishes
 * the device only for a layout it validated in that form. That is why the record carries no
 * bit-depth, visual, timing, or margin field, and why it carries no physical address: the mapping
 * is the only way to the memory.
 *
 * The kernel side is defined in `kernel/fbdev` and `kernel/syscall`; this header is the userspace
 * half of the same hand-maintained contract. Layout changes must be made on both sides together.
 */

/* Reports a `struct roxy_framebuffer_info` into the argument. */
#define ROXY_FRAMEBUFFER_GET_INFO ROXY_IOCTL_FRAMEBUFFER

/* Takes exclusive control of the visible frame for the calling process.
 *
 * While a process holds the frame, the kernel console does not draw, so it cannot overwrite the
 * client's pixels. Control belongs to the process rather than to one descriptor: any thread of the
 * holder may release it, repeating the request from the holder succeeds so a client can assert
 * ownership, and another process's request fails with `EBUSY`. The argument is ignored.
 */
#define ROXY_FRAMEBUFFER_TAKE_CONTROL (ROXY_IOCTL_FRAMEBUFFER + 1)

/* Releases control of the visible frame taken with `TAKE_CONTROL`.
 *
 * Releasing resumes the console, which clears the screen and returns its cursor to the home cell:
 * the console keeps no text buffer, so output written while the frame was held cannot be
 * repainted. A process that does not hold the frame gets `EINVAL`; exiting without releasing
 * releases the frame implicitly. The argument is ignored.
 */
#define ROXY_FRAMEBUFFER_RELEASE_CONTROL (ROXY_IOCTL_FRAMEBUFFER + 2)

/* The framebuffer layout.
 *
 * `stride` is the byte distance between rows and may exceed `width * 4`; it is the value to advance
 * by when drawing a row. `memory_length` is the byte length of one frame, which is the length to
 * map (the kernel accepts a size up to that value rounded up to a whole page).
 */
typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t memory_length;
	uint8_t red_size;
	uint8_t red_shift;
	uint8_t green_size;
	uint8_t green_shift;
	uint8_t blue_size;
	uint8_t blue_shift;
	uint8_t reserved0;
	uint8_t reserved1;
} roxy_framebuffer_info;

#ifndef ROXY_STATIC_ASSERT
#ifdef __cplusplus
#define ROXY_STATIC_ASSERT(condition, message) static_assert(condition, message)
#else
#define ROXY_STATIC_ASSERT(condition, message) _Static_assert(condition, message)
#endif
#endif

ROXY_STATIC_ASSERT(sizeof(roxy_framebuffer_info) == 24,
                   "roxy_framebuffer_info is a 24-byte wire record");
ROXY_STATIC_ASSERT(_Alignof(roxy_framebuffer_info) == 4, "roxy_framebuffer_info is 4-byte aligned");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, width) == 0, "unexpected width offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, height) == 4, "unexpected height offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, stride) == 8, "unexpected stride offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, memory_length) == 12,
                   "unexpected memory_length offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, red_size) == 16, "unexpected red_size offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, red_shift) == 17, "unexpected red_shift offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, green_size) == 18,
                   "unexpected green_size offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, green_shift) == 19,
                   "unexpected green_shift offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, blue_size) == 20, "unexpected blue_size offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, blue_shift) == 21,
                   "unexpected blue_shift offset");
ROXY_STATIC_ASSERT(offsetof(roxy_framebuffer_info, reserved0) == 22, "unexpected reserved offset");

#endif /* _ROXY_FRAMEBUFFER_DEV_H */
