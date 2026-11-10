/* brabnt — Keychron K6 RGB ANSI keymap (SonixQMK w/ Bluetooth).
 *
 * ABNT2 helper layout, ported from the K3 'brabnt'. The OS must be on br/abnt2;
 * under abnt2 the stock keys already produce ç + dead keys. Customizations:
 *   - CapsLock          -> Esc          (ergonomic, spam-friendly)
 *   - top-left key      -> tap Esc, Shift+tap "  [KC_ESCQUOT] (covers most quote use)
 *   - Fn1 + top-left    -> ' "          (KC_GRV; tap = ', Shift = ")
 *   - number row hold   -> F-keys       (1..0 = F1..F10, - = F11, = = F12)
 *   - \ key             -> \ |          (BR_BSLS / KC_NUBS)
 *   - right-mod slot    -> / ?          (BR_SLSH / KC_INT1); donor key
 *   - Fn1 + \           -> ] }          (KC_BSLS)
 *   - top-right key     -> tap Del, Shift/Super+tap Shift/Super+Insert  [KC_DELPASTE]
 *   - Ctrl + Up/Down    -> PageUp/PageDown   (modifier stripped; key override)
 *   - Shift + Backspace -> Delete            (modifier stripped; key override)
 * Bluetooth: hold Fn1 + Q for 3 s to pair (Q blinks blue, then green once
 *   connected). One host only, no profile switching. To reconnect, slide the
 *   mode switch off -> BT. RGB on/off lives on Fn1 + Tab.
 */
#include "keymap.h"

/* Number-row tap-hold: tap = digit, deliberate hold = F-key.
 *
 * Replaces QMK tap dance, which counted fast repeats of the SAME key as one
 * multi-tap and emitted a single digit — so spamming a number dropped keystrokes.
 * Here each tap emits on its own key release, so a number repeats like any normal
 * key. A hold past TAPPING_TERM swaps in the F-key (fired from matrix_scan_user).
 * Rolling onto another key flushes the pending digit immediately, so fast typing
 * never misfires into an F-key. */
typedef struct { uint16_t tap, hold; } nh_pair_t;
static const nh_pair_t nh_pairs[] = {   // indexed by (keycode - NH_1)
    {KC_1, KC_F1}, {KC_2, KC_F2}, {KC_3, KC_F3}, {KC_4, KC_F4},  {KC_5, KC_F5},
    {KC_6, KC_F6}, {KC_7, KC_F7}, {KC_8, KC_F8}, {KC_9, KC_F9},  {KC_0, KC_F10},
    {KC_MINS, KC_F11}, {KC_EQL, KC_F12},
};
static uint16_t nh_keycode = 0;     // pending NH key (0 = none)
static uint16_t nh_timer   = 0;
static bool     nh_held    = false; // hold (F-key) currently registered

// Emit a still-pending number as a tap (rolled onto another key before timeout).
static void nh_flush_tap(void) {
    if (nh_keycode && !nh_held) tap_code16(nh_pairs[nh_keycode - NH_1].tap);
    nh_keycode = 0;
}

/* Bluetooth pairing (KC_BTPAIR = Fn1 + Q): press selects profile 1, still held
 * after BT_HOLD_MS enters pairing. The chip ignores commands for a moment after a
 * profile switch, so the two must not be sent back to back (pattern from
 * goyamamoto/KeychronK8-QMK). We don't call the iton_bt driver directly — in USB
 * mode the chip is deinit'd and iton_bt_send2 busy-waits forever. Re-emitting the
 * keyboard's BT keycodes through process_record_kb gets its BT-mode gating for free. */
#define BT_HOLD_MS 3000   // ponytail: hold-to-pair threshold; 3s (~stock Keychron)
static uint16_t bt_timer   = 0;
static bool     bt_pending = false;   // KC_BTPAIR held, pairing not sent yet

static void bt_press(uint16_t kc) {   // synthesize a press of an ansi.c BT keycode
    keyrecord_t r  = {0};
    r.event.pressed = true;
    process_record_kb(kc, &r);
}

void matrix_scan_user(void) {
    if (nh_keycode && !nh_held && timer_elapsed(nh_timer) > TAPPING_TERM) {
        register_code16(nh_pairs[nh_keycode - NH_1].hold);
        nh_held = true;
    }
    if (bt_pending && timer_elapsed(bt_timer) > BT_HOLD_MS) {
        bt_press(BT_PAIR);
        bt_pending = false;
    }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE] = LAYOUT_ansi(
        KC_ESCQUOT, NH_1, NH_2, NH_3, NH_4, NH_5, NH_6, NH_7, NH_8, NH_9, NH_0, NH_MINS, NH_EQL, KC_BSPC, KC_DELPASTE,
        KC_TAB,  KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, BR_BSLS, KC_HOME,
        KC_ESC,  KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_PGUP,
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_UP, KC_PGDN,
        KC_LCTL, KC_LGUI, KC_LALT, KC_SPC, BR_SLSH, MO(FN1), MO(FN2), KC_LEFT, KC_DOWN, KC_RGHT),

    [FN1] = LAYOUT_ansi(
        KC_GRV,  KC_BRID, KC_BRIU, KC_TASK, KC_FLXP, RM_VALD, RM_VALU, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, _______, _______,
        RM_TOGG, KC_BTPAIR, _______, _______, _______, _______, _______, _______, _______, _______, KC_INS, KC_DEL, KC_END, KC_BSLS, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, QK_BOOT, KC_RALT, _______, _______, _______, _______, _______),

    [FN2] = LAYOUT_ansi(
        KC_TILD, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, _______, RM_NEXT,
        _______, RGB_M_P, RGB_M_B, RGB_M_R, RGB_M_SW, _______, _______, _______, _______, _______, _______, _______, _______, _______, RM_PREV,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, RM_SPDU,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, RM_SATU, RM_SPDD,
        _______, _______, _______, _______, _______, _______, _______, RM_HUED, RM_SATD, RM_HUEU)};

/* Ctrl+Up/Down -> PageUp/PageDown and Shift+Backspace -> Delete, trigger mod
 * stripped (QMK key overrides). Activates on the key press only, and never falls
 * back to the original key if the mod is released first (no stray Backspace).
 * The two corner keys stay hand-rolled: an override can't tell the top-left Esc
 * from the CapsLock Esc, nor the top-right Del from Fn1+[. */
#define KO(mods, trig, repl) ko_make_with_layers_negmods_and_options(mods, trig, repl, ~0, 0, ko_option_activation_trigger_down | ko_option_no_reregister_trigger)
static const key_override_t ko_pgup = KO(MOD_MASK_CTRL, KC_UP, KC_PGUP);
static const key_override_t ko_pgdn = KO(MOD_MASK_CTRL, KC_DOWN, KC_PGDN);
static const key_override_t ko_del  = KO(MOD_MASK_SHIFT, KC_BSPC, KC_DEL);
const key_override_t *key_overrides[] = {&ko_pgup, &ko_pgdn, &ko_del};

static bool delpaste_ins = false;   // top-right held Insert (Shift -> Shift+Insert)
static bool escquot_grv  = false;   // top-left held KC_GRV (Shift -> ")

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Rolling onto any other key flushes a pending number as a tap (no F misfire).
    // So does releasing a modifier: Shift let go before the digit must still give "!".
    if (nh_keycode && keycode != nh_keycode && !nh_held && (record->event.pressed || IS_MODIFIER_KEYCODE(keycode))) {
        nh_flush_tap();
    }
    switch (keycode) {
        case NH_1 ... NH_EQL:
            if (record->event.pressed) {
                if (nh_held) unregister_code16(nh_pairs[nh_keycode - NH_1].hold);   // another F-key still down
                nh_keycode = keycode;
                nh_timer   = timer_read();
                nh_held    = false;
            } else if (nh_held) {                 // hold released -> drop F-key
                unregister_code16(nh_pairs[keycode - NH_1].hold);
                nh_held    = false;
                nh_keycode = 0;
            } else if (nh_keycode == keycode) {   // released before timeout -> tap
                tap_code16(nh_pairs[keycode - NH_1].tap);
                nh_keycode = 0;
            }
            return false;
        case KC_BTPAIR:
            bt_pending = record->event.pressed;   // release before BT_HOLD_MS cancels
            if (bt_pending) {
                bt_timer = timer_read();
                bt_press(BT_PROFILE1);
            }
            return false;
        case KC_DELPASTE:
            if (record->event.pressed) {
                if (get_mods() & (MOD_MASK_SHIFT | MOD_MASK_GUI)) {
                    delpaste_ins = true;
                    register_code(KC_INS);   // mod kept -> host sees Shift+Insert / Super+Insert
                } else {
                    register_code(KC_DEL);
                }
            } else if (delpaste_ins) {
                delpaste_ins = false;
                unregister_code(KC_INS);
            } else {
                unregister_code(KC_DEL);
            }
            return false;
        case KC_ESCQUOT:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) {
                    escquot_grv = true;
                    register_code(KC_GRV);   // Shift kept -> host sees Shift+KC_GRV = "
                } else {
                    register_code(KC_ESC);
                }
            } else if (escquot_grv) {
                escquot_grv = false;
                unregister_code(KC_GRV);
            } else {
                unregister_code(KC_ESC);
            }
            return false;
    }
    return true;
}
