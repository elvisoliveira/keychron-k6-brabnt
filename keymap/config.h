#pragma once

/* The board rules.mk sets ITON_BT_ENABLE_ACK = yes, but that make var never
 * becomes a #define, so the driver's ACK handshake (wake/connect/disconnect)
 * compiles out. Define both here where they actually reach the compiler. */
#define ITON_BT_ENABLE_ACK
#define ITON_BT_ENABLE_LED_STATE

/* Minimalist RGB: the keyboard enables only SOLID_COLOR + SOLID_REACTIVE_MULTIWIDE.
 * Swap the reactive one for MULTISPLASH (colourful splashes that appear only on
 * keypress) and default to solid white. The RGB_MOD cycle is then two modes:
 *   1. SOLID_COLOR  (white)        2. MULTISPLASH (colour, on keypress only)
 * Fn1 + top-right toggles on/off; Fn2 + top-right cycles. */
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
#define ENABLE_RGB_MATRIX_MULTISPLASH

#ifndef RGB_MATRIX_KEYPRESSES
#    define RGB_MATRIX_KEYPRESSES
#endif
#ifndef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#    define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#endif

#undef RGB_MATRIX_DEFAULT_MODE
#undef RGB_MATRIX_DEFAULT_HUE
#undef RGB_MATRIX_DEFAULT_SAT
#undef RGB_MATRIX_DEFAULT_VAL
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_COLOR
#define RGB_MATRIX_DEFAULT_HUE 0
#define RGB_MATRIX_DEFAULT_SAT 0
#define RGB_MATRIX_DEFAULT_VAL 50   // dim — calm base for terminal/coding
