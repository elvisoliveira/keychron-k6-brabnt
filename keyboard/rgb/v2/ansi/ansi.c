/*
Copyright 2025 mintyleaf

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quantum.h"
#include "keymap.h"
#include <stdint.h>

#ifdef BLUETOOTH_ENABLE
#    include "iton_bt.h"
#    include "outputselect.h"
#endif

#ifdef BLUETOOTH_ENABLE
/* Pairing helper: the only LED logic here. While a BT command is pending, the
 * keyboard goes dark and the profile key (Q/W/E) shows the state:
 *   hold to pair    -> blinks blue   until connected or 60 s
 *   connected       -> solid green   for 2 s
 * Works with RGB off too: the matrix is enabled without touching EEPROM and
 * switched back off afterwards. Chip "disconnected" events are ignored, so they
 * can't cut the pairing blink short. */
#    define BT_PROFILE_LED_START_INDEX 16 // Q; W and E follow
#    define BT_PAIR_MS 60000              // ponytail: guessed windows, the chip's own may differ
#    define BT_CONNECTED_MS 2000
#    define BT_BLINK_MS 250

typedef enum { BT_LED_IDLE, BT_LED_PAIRING, BT_LED_CONNECTED } bt_led_t;

static bt_led_t bt_led          = BT_LED_IDLE;
static uint16_t bt_led_timer    = 0;
static uint16_t bt_led_ms       = 0;
static bool     bt_led_borrowed = false;
static uint8_t  bt_profile      = 0;

static bool bluetooth_dip_switch = false;
static volatile bool bt_send_no_sleep = false; // set in the chip's IRQ, sent from housekeeping

static void bt_led_set(bt_led_t state, uint16_t ms) {
    bt_led       = state;
    bt_led_ms    = ms;
    bt_led_timer = timer_read();
    if (!rgb_matrix_is_enabled()) {
        rgb_matrix_enable_noeeprom();
        bt_led_borrowed = true;
    }
}

// Runs inside the chip's IRQ: never send to the chip from here. The send would
// spin on the SPI line that only the next interrupt releases, freezing the MCU.
void iton_bt_connection_successful() {
    set_output(OUTPUT_BLUETOOTH);
    bt_send_no_sleep = true;
    bt_led_set(BT_LED_CONNECTED, BT_CONNECTED_MS);
}

void housekeeping_task_kb(void) {
    if (bt_send_no_sleep && bluetooth_dip_switch) {
        bt_send_no_sleep = false;
        // ponytail: never let the ITON chip idle-sleep — kills the lost/laggy first
        // key after idle; costs battery. Swap for sleep_idle_30m if that matters.
        iton_bt_disable_sleep();
    }
    housekeeping_task_user();
}
#endif

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
#ifdef BLUETOOTH_ENABLE
    if (record->event.pressed && bluetooth_dip_switch) {
        switch (keycode) {
            case BT_PROFILE1:
            case BT_PROFILE2:
            case BT_PROFILE3:
                bt_profile = keycode - BT_PROFILE1;
                iton_bt_switch_profile(bt_profile);
                return false;
            case BT_PAIR:
                iton_bt_enter_pairing();
                bt_led_set(BT_LED_PAIRING, BT_PAIR_MS);
                return false;
            default:
                break;
        }
    }
#endif
    switch (keycode) {
        case KC_MISSION_CONTROL:
            host_consumer_send(record->event.pressed ? 0x29F : 0);
            return false;
        case KC_LAUNCHPAD:
            host_consumer_send(record->event.pressed ? 0x2A0 : 0);
            return false;
        default:
            break;
    }
    return process_record_user(keycode, record);
}

#ifdef BLUETOOTH_ENABLE
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (bt_led == BT_LED_IDLE) return true;

    if (!bluetooth_dip_switch || timer_elapsed(bt_led_timer) > bt_led_ms) {
        bt_led = BT_LED_IDLE;
        if (bt_led_borrowed) {
            bt_led_borrowed = false;
            rgb_matrix_disable_noeeprom();
        }
        return true;
    }

    rgb_matrix_set_color_all(0, 0, 0);
    uint8_t i = BT_PROFILE_LED_START_INDEX + bt_profile;
    if (bt_led == BT_LED_CONNECTED) {
        rgb_matrix_set_color(i, RGB_GREEN);
    } else if ((timer_read() / BT_BLINK_MS) % 2) {
        rgb_matrix_set_color(i, RGB_BLUE);
    }
    return true;
}
#endif

bool dip_switch_update_user(uint8_t index, bool active) {
    switch (index) {
        case 1:
            layer_move(active ? MAC_BASE : WIN_BASE);
            return false;
#ifdef BLUETOOTH_ENABLE
        case 0:
            // dip switch is inactive in bt state
            if (!active) {
                set_output(OUTPUT_NONE);
                iton_bt_init();
            } else {
                iton_bt_deinit();
                set_output(OUTPUT_USB);
            }
            bluetooth_dip_switch = !active;
            return false;
#endif
    }
    return true;
}
