#include <stdint.h>
#include "led_ctrl.h"
#include "pico/stdlib.h"

static const uint32_t LED_MASKS[5] = {
    LED_USER,
    LED_GOLIOTH,
    LED_INTERNET,
    LED_BATTERY,
    LED_POWER
};

void led_init(void) {
    gpio_init_mask(LED_ALL_MASK);
    gpio_set_dir_out_masked(LED_ALL_MASK);
}

void led_push_mask(uint8_t mask) {
    uint32_t on_mask = 0;
    uint32_t off_mask = 0;

    for (uint8_t i=0; i<5; i++) {
        if (mask & 1<<i) {
            on_mask |= LED_MASKS[i];
        } else {
            off_mask |= LED_MASKS[i];
        }
    }

    // LEDs are active low so CLR turns them on
    gpio_clr_mask(on_mask);
    // SET turns them off
    gpio_set_mask(off_mask);
}

void led_push_single(uint32_t led_bit, uint8_t state) {
    if (state) {
        gpio_clr_mask(led_bit);
    } else {
        gpio_set_mask(led_bit);
    }
}

