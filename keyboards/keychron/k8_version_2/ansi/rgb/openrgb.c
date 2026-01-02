/* Copyright 2020 Kasper
 * Copyright 2025 Modified for Keychron K8 V2
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "openrgb.h"
#include "raw_hid.h"
#include "version.h"
#include "rgb_matrix.h"
#include <string.h>

#ifdef LK_WIRELESS_ENABLE
#    include "transport.h"
#    if defined(PROTOCOL_CHIBIOS)
#        include <usb_main.h>
#    endif
#endif

// Direct mode color buffer for all LEDs
RGB g_openrgb_direct_mode_colors[RGB_MATRIX_LED_COUNT] = {
    [0 ... RGB_MATRIX_LED_COUNT - 1] = {
        OPENRGB_DIRECT_MODE_STARTUP_RED,    // .r member
        OPENRGB_DIRECT_MODE_STARTUP_GREEN,  // .g member
        OPENRGB_DIRECT_MODE_STARTUP_BLUE    // .b member
    }
};

// HID buffer for responses
static uint8_t raw_hid_buffer[RAW_EPSIZE];

// List of enabled RGB matrix effect indices for OpenRGB
static const uint8_t openrgb_rgb_matrix_effects_indexes[] = {
    1,  // RGB_MATRIX_SOLID_COLOR
    2,  // RGB_MATRIX_ALPHAS_MODS (OpenRGB Direct Mode)
#ifdef ENABLE_RGB_MATRIX_BREATHING
    6,
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_ALL
    13,
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
    14,
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_UP_DOWN
    15,
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
    18,
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
    19,
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_SPIRAL
    20,
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_BEACON
    22,
#endif
#ifdef ENABLE_RGB_MATRIX_DIGITAL_RAIN
    30,
#endif
#ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
    31,
#endif
#ifdef ENABLE_RGB_MATRIX_SPLASH
    39,
#endif
};

// Check if OpenRGB should be active (USB cable connected, regardless of transport mode)
bool openrgb_is_active(void) {
#ifdef LK_WIRELESS_ENABLE
    // Check if USB is physically connected, not just if it's the active transport
    // This allows OpenRGB to work when keyboard is plugged in but using Bluetooth
    return USB_DRIVER.state == USB_ACTIVE;
#else
    return true;
#endif
}

// OpenRGB HID receive handler for VIA hybrid mode
void orgb_raw_hid_receive(uint8_t *data, uint8_t length) {
    // USB check is done by the caller (via.c)
    // Just process the OpenRGB command

    // Clear the buffer
    memset(raw_hid_buffer, 0x00, RAW_EPSIZE);

    switch (data[0]) {
        case OPENRGB_GET_PROTOCOL_VERSION:
            openrgb_get_protocol_version();
            break;
        case OPENRGB_GET_QMK_VERSION:
            openrgb_get_qmk_version();
            break;
        case OPENRGB_GET_DEVICE_INFO:
            openrgb_get_device_info();
            break;
        case OPENRGB_GET_MODE_INFO:
            openrgb_get_mode_info();
            break;
        case OPENRGB_GET_LED_INFO:
            openrgb_get_led_info(data);
            break;
        case OPENRGB_GET_ENABLED_MODES:
            openrgb_get_enabled_modes();
            break;
        case OPENRGB_SET_MODE:
            openrgb_set_mode(data);
            break;
        case OPENRGB_DIRECT_MODE_SET_SINGLE_LED:
            openrgb_direct_mode_set_single_led(data);
            break;
        case OPENRGB_DIRECT_MODE_SET_LEDS:
            openrgb_direct_mode_set_leds(data);
            break;
        default:
            // Unknown command
            raw_hid_buffer[0] = data[0];
            raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_FAILURE;
            break;
    }

    // Send response (except for bulk LED updates which don't need immediate response)
    if (data[0] != OPENRGB_DIRECT_MODE_SET_LEDS) {
        raw_hid_buffer[RAW_EPSIZE - 1] = OPENRGB_END_OF_MESSAGE;
        raw_hid_send(raw_hid_buffer, RAW_EPSIZE);
    }
}

void openrgb_get_protocol_version(void) {
    raw_hid_buffer[0] = OPENRGB_GET_PROTOCOL_VERSION;
    raw_hid_buffer[1] = OPENRGB_PROTOCOL_VERSION;
}

void openrgb_get_qmk_version(void) {
    raw_hid_buffer[0] = OPENRGB_GET_QMK_VERSION;
    uint8_t idx = 1;
    for (uint8_t i = 0; (idx < (RAW_EPSIZE - 2)) && (QMK_VERSION[i] != 0); i++) {
        raw_hid_buffer[idx++] = QMK_VERSION[i];
    }
}

void openrgb_get_device_info(void) {
    raw_hid_buffer[0] = OPENRGB_GET_DEVICE_INFO;
    raw_hid_buffer[1] = RGB_MATRIX_LED_COUNT;
    raw_hid_buffer[2] = MATRIX_COLS * MATRIX_ROWS;

    // Add product name
    uint8_t idx = 3;
#ifdef PRODUCT
    const char* product = PRODUCT;
    for (uint8_t i = 0; (idx < ((RAW_EPSIZE - 2) / 2)) && (product[i] != 0); i++) {
        raw_hid_buffer[idx++] = product[i];
    }
#else
    const char* product = "Keychron K8 V2";
    for (uint8_t i = 0; (idx < ((RAW_EPSIZE - 2) / 2)) && (product[i] != 0); i++) {
        raw_hid_buffer[idx++] = product[i];
    }
#endif
    raw_hid_buffer[idx++] = 0; // null terminator

    // Add manufacturer name
#ifdef MANUFACTURER
    const char* manufacturer = MANUFACTURER;
    for (uint8_t i = 0; (idx + 2 < RAW_EPSIZE) && (manufacturer[i] != 0); i++) {
        raw_hid_buffer[idx++] = manufacturer[i];
    }
#else
    const char* manufacturer = "Keychron";
    for (uint8_t i = 0; (idx + 2 < RAW_EPSIZE) && (manufacturer[i] != 0); i++) {
        raw_hid_buffer[idx++] = manufacturer[i];
    }
#endif
}

void openrgb_get_mode_info(void) {
    HSV hsv_color = rgb_matrix_get_hsv();

    raw_hid_buffer[0] = OPENRGB_GET_MODE_INFO;
    raw_hid_buffer[1] = rgb_matrix_get_mode();
    raw_hid_buffer[2] = rgb_matrix_get_speed();
    raw_hid_buffer[3] = hsv_color.h;
    raw_hid_buffer[4] = hsv_color.s;
    raw_hid_buffer[5] = hsv_color.v;
}

void openrgb_get_led_info(uint8_t *data) {
    const uint8_t first_led   = data[1];
    const uint8_t number_leds = data[2];

    raw_hid_buffer[0] = OPENRGB_GET_LED_INFO;

    for (uint8_t i = 0; i < number_leds; i++) {
        const uint8_t led_idx  = first_led + i;
        const uint8_t data_idx = i * 7;

        // Prevent buffer overflow - each LED needs 7 bytes
        if (data_idx + 7 >= RAW_EPSIZE - 1) {
            break;  // Stop if we would overflow the buffer
        }

        if (led_idx >= RGB_MATRIX_LED_COUNT) {
            raw_hid_buffer[data_idx + 3] = OPENRGB_FAILURE;
        } else {
            raw_hid_buffer[data_idx + 1] = g_led_config.point[led_idx].x;
            raw_hid_buffer[data_idx + 2] = g_led_config.point[led_idx].y;
            raw_hid_buffer[data_idx + 3] = g_led_config.flags[led_idx];
            raw_hid_buffer[data_idx + 4] = g_openrgb_direct_mode_colors[led_idx].r;
            raw_hid_buffer[data_idx + 5] = g_openrgb_direct_mode_colors[led_idx].g;
            raw_hid_buffer[data_idx + 6] = g_openrgb_direct_mode_colors[led_idx].b;

            // Find the keycode for this LED
            uint8_t row = 0, col = 0;
            bool found = false;
            for (row = 0; row < MATRIX_ROWS && !found; row++) {
                for (col = 0; col < MATRIX_COLS; col++) {
                    if (g_led_config.matrix_co[row][col] == led_idx) {
                        found = true;
                        break;
                    }
                }
            }
            raw_hid_buffer[data_idx + 7] = found ? keymap_key_to_keycode(0, MAKE_KEYPOS(row - 1, col)) : KC_NO;
        }
    }
}

void openrgb_get_enabled_modes(void) {
    raw_hid_buffer[0] = OPENRGB_GET_ENABLED_MODES;
    const uint8_t size = sizeof(openrgb_rgb_matrix_effects_indexes) / sizeof(openrgb_rgb_matrix_effects_indexes[0]);
    for (uint8_t i = 0; i < size && (i + 1) < RAW_EPSIZE - 2; i++) {
        raw_hid_buffer[i + 1] = openrgb_rgb_matrix_effects_indexes[i];
    }
}

void openrgb_set_mode(uint8_t *data) {
    const uint8_t h     = data[1];
    const uint8_t s     = data[2];
    const uint8_t v     = data[3];
    const uint8_t mode  = data[4];
    const uint8_t speed = data[5];
    const uint8_t save  = data[6];

    raw_hid_buffer[0] = OPENRGB_SET_MODE;

    if (mode >= RGB_MATRIX_EFFECT_MAX) {
        raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_FAILURE;
        return;
    }

    // Disable direct mode when switching to a QMK effect
    openrgb_set_direct_mode(false);

    if (save == 1) {
        rgb_matrix_mode(mode);
        rgb_matrix_set_speed(speed);
        rgb_matrix_sethsv(h, s, v);
    } else {
        rgb_matrix_mode_noeeprom(mode);
        rgb_matrix_set_speed_noeeprom(speed);
        rgb_matrix_sethsv_noeeprom(h, s, v);
    }

    raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_SUCCESS;
}

void openrgb_direct_mode_set_single_led(uint8_t *data) {
    const uint8_t led = data[1];
    const uint8_t r   = data[2];
    const uint8_t g   = data[3];
    const uint8_t b   = data[4];

    raw_hid_buffer[0] = OPENRGB_DIRECT_MODE_SET_SINGLE_LED;

    if (led >= RGB_MATRIX_LED_COUNT) {
        raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_FAILURE;
        return;
    }

    // Enable direct mode when receiving LED color data
    openrgb_set_direct_mode(true);

    g_openrgb_direct_mode_colors[led].r = r;
    g_openrgb_direct_mode_colors[led].g = g;
    g_openrgb_direct_mode_colors[led].b = b;

    raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_SUCCESS;
}

void openrgb_direct_mode_set_leds(uint8_t *data) {
    const uint8_t first_led   = data[1];
    const uint8_t number_leds = data[2];

    // Enable direct mode when receiving LED color data
    openrgb_set_direct_mode(true);

    for (uint8_t i = 0; i < number_leds; i++) {
        const uint8_t led_idx  = first_led + i;
        const uint8_t data_idx = i * 3;

        if (led_idx < RGB_MATRIX_LED_COUNT) {
            g_openrgb_direct_mode_colors[led_idx].r = data[data_idx + 3];
            g_openrgb_direct_mode_colors[led_idx].g = data[data_idx + 4];
            g_openrgb_direct_mode_colors[led_idx].b = data[data_idx + 5];
        }
    }
    // No response sent for bulk updates (performance)
}
