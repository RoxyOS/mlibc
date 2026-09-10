#ifndef _ROXY_MOUSE_DEV_H
#define _ROXY_MOUSE_DEV_H

#include <stddef.h>
#include <stdint.h>

/* Roxy's mouse device protocol.
 *
 * `/dev/mouse` is a read-only character device that serves a stream of fixed-size
 * `roxy_mouse_event` records: exactly one record per hardware sample. There is no ioctl
 * interface; a reader consumes whole records in a loop and uses the return value of `read` to
 * find how many arrived. A buffer shorter than one record is rejected with EINVAL rather than
 * split across two calls.
 *
 * `buttons` is the button state *after* the sample, not a change mask: a reader diffs successive
 * records to find transitions, and a reader that starts mid-stream has the whole state in its
 * first record.
 *
 * The kernel side is defined in `kernel/mouse-dev`; this header is the userspace half of the
 * same hand-maintained contract. Layout changes must be made on both sides together.
 */

/* Button bits of `roxy_mouse_event.buttons`. */
#define ROXY_MOUSE_BTN_LEFT (1u << 0)
#define ROXY_MOUSE_BTN_RIGHT (1u << 1)
#define ROXY_MOUSE_BTN_MIDDLE (1u << 2)

/* One hardware sample, as read from `/dev/mouse`.
 *
 * `dx`/`dy` are relative motion to the right and downward in hardware counts; `wheel` is the
 * wheel steps of this sample, positive upward, and is always zero on a mouse without a wheel.
 */
typedef struct {
	uint64_t timestamp_ns;
	int32_t dx;
	int32_t dy;
	int32_t wheel;
	uint32_t buttons;
} roxy_mouse_event;

#ifndef ROXY_STATIC_ASSERT
#ifdef __cplusplus
#define ROXY_STATIC_ASSERT(condition, message) static_assert(condition, message)
#else
#define ROXY_STATIC_ASSERT(condition, message) _Static_assert(condition, message)
#endif
#endif

ROXY_STATIC_ASSERT(sizeof(roxy_mouse_event) == 24, "roxy_mouse_event is a 24-byte wire record");
ROXY_STATIC_ASSERT(_Alignof(roxy_mouse_event) == 8, "roxy_mouse_event must be 8-byte aligned");
ROXY_STATIC_ASSERT(offsetof(roxy_mouse_event, timestamp_ns) == 0, "unexpected timestamp offset");
ROXY_STATIC_ASSERT(offsetof(roxy_mouse_event, dx) == 8, "unexpected dx offset");
ROXY_STATIC_ASSERT(offsetof(roxy_mouse_event, dy) == 12, "unexpected dy offset");
ROXY_STATIC_ASSERT(offsetof(roxy_mouse_event, wheel) == 16, "unexpected wheel offset");
ROXY_STATIC_ASSERT(offsetof(roxy_mouse_event, buttons) == 20, "unexpected buttons offset");

#endif /* _ROXY_MOUSE_DEV_H */
