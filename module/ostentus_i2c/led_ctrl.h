#ifndef _LED_CTRL_H_
#define _LED_CTRL_H_

#include <stdint.h>

// These should match the register address here:
// https://github.com/golioth/ostentus/blob/8b73f306b55f31b6a26226b7cdb5381ca2dc138f/zephyr/ostentus.c#L23-L28
#define ADDR_POWER 0x14
#define ADDR_BATTERY 0x13
#define ADDR_INTERNET 0x12
#define ADDR_GOLIOTH 0x11
#define ADDR_USER 0x10
#define ADDR_BITMASK 0x18

#define LED_POWER 1<<15
#define LED_BATTERY 1<<14
#define LED_INTERNET 1<<13
#define LED_GOLIOTH 1<<12
#define LED_USER 1<<11
#define LED_ALL_MASK (LED_POWER | LED_BATTERY | LED_INTERNET | LED_GOLIOTH | LED_USER)

void led_init(void);
void led_push_mask(uint8_t mask);
void led_push_single(uint32_t led_bit, uint8_t state);

#endif
