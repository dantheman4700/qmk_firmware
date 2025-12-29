#include "quantum.h"
#include "hybrid_switch_animation.h"
#include "rgb_matrix.h"

#ifdef RGB_MATRIX_ENABLE

#define ANI_GROWING_INTERVAL 300
#define ANI_ON_INTERVAL 2000

enum {
    ANI_NONE,
    ANI_GROWING,
    ANI_BLINK_OFF,
    ANI_BLINK_ON,
};

static bool cur_is_switch_orgb;
static uint8_t animation_state = ANI_NONE;
static uint32_t ani_timer_buffer = 0;
static uint8_t cur_led;
static uint32_t time_interval;
static uint8_t r, g, b;

void switch_animation_start(bool is_switch_orgb) {
    cur_is_switch_orgb = is_switch_orgb;
    animation_state = ANI_GROWING;
    ani_timer_buffer = timer_read32();
    cur_led = 0;
    time_interval = ANI_GROWING_INTERVAL;
    r = g = b = 0;
}

void switch_animation_stop(void) {
    animation_state = ANI_NONE;
}

bool switch_animation_isactive(void) {
    return animation_state != ANI_NONE;
}

void switch_animation_indicate(void) {
    if (animation_state == ANI_NONE) return;

    // Clear all LEDs logic should be handled by the effect runner if necessary,
    // but here we just overwrite the first few keys.
    // Ideally we want to blank the whole keyboard for clarity.
    rgb_matrix_set_color_all(0, 0, 0);

    if (animation_state == ANI_GROWING) {
        if(cur_is_switch_orgb){
            switch(cur_led) {
                case 0: r = 255; g = 0; b = 0; break;
                case 1: r = 0; g = 255; b = 0; break;
                default: r = 0; g = 0; b = 255; break;
            }
        } else {
            r = 255; g = 166; b = 173; // VIA Pink
        }
        rgb_matrix_set_color(cur_led, r, g, b);
        
        // Keep previous LEDs lit for growing effect
        for(uint8_t i=0; i<cur_led; i++) {
             if(cur_is_switch_orgb){
                switch(i) {
                    case 0: rgb_matrix_set_color(i, 255, 0, 0); break;
                    case 1: rgb_matrix_set_color(i, 0, 255, 0); break;
                    default: rgb_matrix_set_color(i, 0, 0, 255); break;
                }
            } else {
                rgb_matrix_set_color(i, 255, 166, 173);
            }
        }
    }
    else if (animation_state == ANI_BLINK_ON) {
        for (uint8_t i = 0; i < 3; i++) {
            if(cur_is_switch_orgb){
                switch(i) {
                    case 0: rgb_matrix_set_color(i, 255, 0, 0); break;
                    case 1: rgb_matrix_set_color(i, 0, 255, 0); break;
                    default: rgb_matrix_set_color(i, 0, 0, 255); break;
                }
            } else {
                rgb_matrix_set_color(i, 255, 166, 173);
            }
        }
    }
}

static void switch_animation_update(void) {
    switch (animation_state) {
        case ANI_GROWING:
            if (cur_led < 2)
                cur_led += 1;
            else {
                if (cur_led == 0) cur_led = 2; // Safety
                animation_state = ANI_BLINK_OFF;
                time_interval = 200; // Short blink off
            }
            break;

        case ANI_BLINK_OFF:
            time_interval = ANI_ON_INTERVAL;
            animation_state = ANI_BLINK_ON;
            break;

        case ANI_BLINK_ON:
            animation_state = ANI_NONE;
            break;
            
        default:
            break;
    }
    ani_timer_buffer = timer_read32();
}

void switch_animation_task(void) {
    if (animation_state != ANI_NONE && timer_elapsed32(ani_timer_buffer) > time_interval) {
        switch_animation_update();
    }
}

#endif
