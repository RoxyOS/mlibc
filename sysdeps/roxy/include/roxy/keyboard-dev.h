#ifndef _ROXY_KEYBOARD_DEV_H
#define _ROXY_KEYBOARD_DEV_H

#include <stddef.h>
#include <stdint.h>

/* Roxy's keyboard device protocol.
 *
 * `/dev/keyboard` is a read-only character device that serves a stream of fixed-size
 * `roxy_key_event` records: one record per key transition. There is no ioctl interface; a reader
 * consumes whole records in a loop and uses the return value of `read` to find how many arrived.
 * A buffer shorter than one record is rejected with EINVAL rather than split across two calls.
 *
 * The kernel side is defined in `kernel/keyboard-dev`; this header is the userspace half of the
 * same hand-maintained contract. Layout or code changes must be made on both sides together.
 */

/* Key codes.
 *
 * The values are Roxy's own, deliberately not the sparse Linux `KEY_*` numbering. Ranges are
 * grouped by key class so unassigned values stay available for later keys; the kernel assigns
 * each `KeyCode` explicitly and both sides must move together.
 *
 *   0x0000        none (reserved, never produced)
 *   0x0001-0x000F control and editing keys
 *   0x0010-0x0029 letters a-z
 *   0x0030-0x0039 digits 0-9
 *   0x003A-0x0045 punctuation and space
 *   0x0050-0x005A modifiers and lock keys
 *   0x0070-0x007D function and system keys
 *   0x008C-0x009F arrow keys and the numeric keypad
 */
#define ROXY_KEY_NONE 0x0000

#define ROXY_KEY_ESCAPE 0x0001
#define ROXY_KEY_BACKSPACE 0x0002
#define ROXY_KEY_TAB 0x0003
#define ROXY_KEY_ENTER 0x0004
#define ROXY_KEY_INSERT 0x0005
#define ROXY_KEY_DELETE 0x0006
#define ROXY_KEY_HOME 0x0007
#define ROXY_KEY_END 0x0008
#define ROXY_KEY_PAGE_UP 0x0009
#define ROXY_KEY_PAGE_DOWN 0x000A
#define ROXY_KEY_MENU 0x000B

#define ROXY_KEY_A 0x0010
#define ROXY_KEY_B 0x0011
#define ROXY_KEY_C 0x0012
#define ROXY_KEY_D 0x0013
#define ROXY_KEY_E 0x0014
#define ROXY_KEY_F 0x0015
#define ROXY_KEY_G 0x0016
#define ROXY_KEY_H 0x0017
#define ROXY_KEY_I 0x0018
#define ROXY_KEY_J 0x0019
#define ROXY_KEY_K 0x001A
#define ROXY_KEY_L 0x001B
#define ROXY_KEY_M 0x001C
#define ROXY_KEY_N 0x001D
#define ROXY_KEY_O 0x001E
#define ROXY_KEY_P 0x001F
#define ROXY_KEY_Q 0x0020
#define ROXY_KEY_R 0x0021
#define ROXY_KEY_S 0x0022
#define ROXY_KEY_T 0x0023
#define ROXY_KEY_U 0x0024
#define ROXY_KEY_V 0x0025
#define ROXY_KEY_W 0x0026
#define ROXY_KEY_X 0x0027
#define ROXY_KEY_Y 0x0028
#define ROXY_KEY_Z 0x0029

#define ROXY_KEY_0 0x0030
#define ROXY_KEY_1 0x0031
#define ROXY_KEY_2 0x0032
#define ROXY_KEY_3 0x0033
#define ROXY_KEY_4 0x0034
#define ROXY_KEY_5 0x0035
#define ROXY_KEY_6 0x0036
#define ROXY_KEY_7 0x0037
#define ROXY_KEY_8 0x0038
#define ROXY_KEY_9 0x0039

#define ROXY_KEY_GRAVE 0x003A
#define ROXY_KEY_MINUS 0x003B
#define ROXY_KEY_EQUALS 0x003C
#define ROXY_KEY_BRACKET_LEFT 0x003D
#define ROXY_KEY_BRACKET_RIGHT 0x003E
#define ROXY_KEY_BACKSLASH 0x003F
#define ROXY_KEY_SEMICOLON 0x0040
#define ROXY_KEY_APOSTROPHE 0x0041
#define ROXY_KEY_COMMA 0x0042
#define ROXY_KEY_PERIOD 0x0043
#define ROXY_KEY_SLASH 0x0044
#define ROXY_KEY_SPACE 0x0045

#define ROXY_KEY_LEFT_SHIFT 0x0050
#define ROXY_KEY_RIGHT_SHIFT 0x0051
#define ROXY_KEY_LEFT_CTRL 0x0052
#define ROXY_KEY_RIGHT_CTRL 0x0053
#define ROXY_KEY_LEFT_ALT 0x0054
#define ROXY_KEY_RIGHT_ALT 0x0055
#define ROXY_KEY_LEFT_SUPER 0x0056
#define ROXY_KEY_RIGHT_SUPER 0x0057
#define ROXY_KEY_CAPS_LOCK 0x0058
#define ROXY_KEY_SCROLL_LOCK 0x0059
#define ROXY_KEY_NUM_LOCK 0x005A

#define ROXY_KEY_F1 0x0070
#define ROXY_KEY_F2 0x0071
#define ROXY_KEY_F3 0x0072
#define ROXY_KEY_F4 0x0073
#define ROXY_KEY_F5 0x0074
#define ROXY_KEY_F6 0x0075
#define ROXY_KEY_F7 0x0076
#define ROXY_KEY_F8 0x0077
#define ROXY_KEY_F9 0x0078
#define ROXY_KEY_F10 0x0079
#define ROXY_KEY_F11 0x007A
#define ROXY_KEY_F12 0x007B
#define ROXY_KEY_PRINT_SCREEN 0x007C
#define ROXY_KEY_PAUSE 0x007D

#define ROXY_KEY_UP 0x008C
#define ROXY_KEY_DOWN 0x008D
#define ROXY_KEY_LEFT 0x008E
#define ROXY_KEY_RIGHT 0x008F
#define ROXY_KEY_KP_DIVIDE 0x0090
#define ROXY_KEY_KP_MULTIPLY 0x0091
#define ROXY_KEY_KP_SUBTRACT 0x0092
#define ROXY_KEY_KP_ADD 0x0093
#define ROXY_KEY_KP_ENTER 0x0094
#define ROXY_KEY_KP_DECIMAL 0x0095
#define ROXY_KEY_KP_0 0x0096
#define ROXY_KEY_KP_1 0x0097
#define ROXY_KEY_KP_2 0x0098
#define ROXY_KEY_KP_3 0x0099
#define ROXY_KEY_KP_4 0x009A
#define ROXY_KEY_KP_5 0x009B
#define ROXY_KEY_KP_6 0x009C
#define ROXY_KEY_KP_7 0x009D
#define ROXY_KEY_KP_8 0x009E
#define ROXY_KEY_KP_9 0x009F

/* One key transition, as read from `/dev/keyboard`.
 *
 * `pressed` is 1 for a press and 0 for a release. The two reserved fields are padding the kernel
 * always writes as zero; they keep the record free of implicit padding so its 16-byte size is
 * the wire size.
 */
typedef struct {
	uint64_t timestamp_ns;
	uint16_t code;
	uint8_t pressed;
	uint8_t reserved;
	uint32_t reserved2;
} roxy_key_event;

#ifndef ROXY_STATIC_ASSERT
#ifdef __cplusplus
#define ROXY_STATIC_ASSERT(condition, message) static_assert(condition, message)
#else
#define ROXY_STATIC_ASSERT(condition, message) _Static_assert(condition, message)
#endif
#endif

ROXY_STATIC_ASSERT(sizeof(roxy_key_event) == 16, "roxy_key_event is a 16-byte wire record");
ROXY_STATIC_ASSERT(_Alignof(roxy_key_event) == 8, "roxy_key_event must be 8-byte aligned");
ROXY_STATIC_ASSERT(offsetof(roxy_key_event, timestamp_ns) == 0, "unexpected timestamp offset");
ROXY_STATIC_ASSERT(offsetof(roxy_key_event, code) == 8, "unexpected code offset");
ROXY_STATIC_ASSERT(offsetof(roxy_key_event, pressed) == 10, "unexpected pressed offset");
ROXY_STATIC_ASSERT(offsetof(roxy_key_event, reserved) == 11, "unexpected reserved offset");
ROXY_STATIC_ASSERT(offsetof(roxy_key_event, reserved2) == 12, "unexpected reserved2 offset");

#endif /* _ROXY_KEYBOARD_DEV_H */
