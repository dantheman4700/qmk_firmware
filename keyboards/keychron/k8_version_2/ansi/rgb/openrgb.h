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

#pragma once

#include "quantum.h"

// Protocol version - must match OpenRGB expectations
#define OPENRGB_PROTOCOL_VERSION 0x0C

// HID endpoint size - 64 bytes for OpenRGB compatibility
#ifndef RAW_EPSIZE
#    define RAW_EPSIZE 64
#endif

// Default startup colors for direct mode
#ifndef OPENRGB_DIRECT_MODE_STARTUP_RED
#    define OPENRGB_DIRECT_MODE_STARTUP_RED 0x00
#endif
#ifndef OPENRGB_DIRECT_MODE_STARTUP_GREEN
#    define OPENRGB_DIRECT_MODE_STARTUP_GREEN 0x00
#endif
#ifndef OPENRGB_DIRECT_MODE_STARTUP_BLUE
#    define OPENRGB_DIRECT_MODE_STARTUP_BLUE 0x00
#endif

// Command IDs from OpenRGB
enum openrgb_command_id {
    OPENRGB_GET_PROTOCOL_VERSION = 1,
    OPENRGB_GET_QMK_VERSION,
    OPENRGB_GET_DEVICE_INFO,
    OPENRGB_GET_MODE_INFO,
    OPENRGB_GET_LED_INFO,
    OPENRGB_GET_ENABLED_MODES,

    OPENRGB_SET_MODE,
    OPENRGB_DIRECT_MODE_SET_SINGLE_LED,
    OPENRGB_DIRECT_MODE_SET_LEDS,
};

// Response codes
enum openrgb_responses {
    OPENRGB_FAILURE        = 25,
    OPENRGB_SUCCESS        = 50,
    OPENRGB_END_OF_MESSAGE = 100,
};

// Direct mode color buffer (defined in openrgb.c)
extern RGB g_openrgb_direct_mode_colors[];

// Protocol functions
void openrgb_get_protocol_version(void);
void openrgb_get_qmk_version(void);
void openrgb_get_device_info(void);
void openrgb_get_mode_info(void);
void openrgb_get_led_info(uint8_t *data);
void openrgb_get_enabled_modes(void);

void openrgb_set_mode(uint8_t *data);
void openrgb_direct_mode_set_single_led(uint8_t *data);
void openrgb_direct_mode_set_leds(uint8_t *data);

// Check if OpenRGB should be active (USB connected)
bool openrgb_is_active(void);

// Raw HID handler for OpenRGB protocol (called by VIA dispatch)
void orgb_raw_hid_receive(uint8_t *data, uint8_t length);

// Direct mode tracking (implemented in rgb.c)
void openrgb_set_direct_mode(bool active);
bool openrgb_get_direct_mode(void);
