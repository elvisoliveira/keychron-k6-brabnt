/* brabnt — Keychron K6 RGB ANSI (SonixQMK w/ Bluetooth), ABNT2 helpers.
 *
 * This header is included by keyboard/rgb/v2/ansi/ansi.c (which references
 * BT_keycodes and the layer names). Our own custom keycodes start at K6_SAFE_RANGE.
 */
#include QMK_KEYBOARD_H

// Bluetooth keycodes handled by ansi.c. Do not reorder.
enum BT_keycodes { BT_PROFILE1 = SAFE_RANGE, BT_PROFILE2, BT_PROFILE3, BT_PAIR, BT_TOGGLE, BT_RESET, BT_BATTERY, K6_SAFE_RANGE };

// brabnt custom keycodes (after the BT range).
// top-right: tap=Del, Shift/Super=Shift+Insert/Super+Insert. top-left: tap=Esc, Shift=" (Shift+KC_GRV under abnt2).
// NH_*: number-row tap-hold (tap = digit, hold = F-key). NH_1..NH_EQL MUST stay contiguous
// and in row order — the handler indexes nh_pairs[] by (keycode - NH_1).
// KC_BTPAIR: hold 3 s to pair (profile 1 only).
enum custom_keycodes {
    KC_DELPASTE = K6_SAFE_RANGE, KC_ESCQUOT,
    NH_1, NH_2, NH_3, NH_4, NH_5, NH_6, NH_7, NH_8, NH_9, NH_0, NH_MINS, NH_EQL,
    KC_BTPAIR
};

enum layer_names { BASE, FN1, FN2 };

#define KC_TASK LGUI(KC_TAB)        // Task viewer
#define KC_FLXP LGUI(KC_E)          // Windows file explorer

#define BR_SLSH KC_INT1   // under abnt2 => / ?
#define BR_BSLS KC_NUBS   // under abnt2 => \ |
