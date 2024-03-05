#ifndef _LED_CTRL_H_
#define _LED_CTRL_H_

#define LED_POWER 1<<15
#define LED_BATTERY 1<<14
#define LED_INTERNET 1<<13
#define LED_GOLIOTH_R 1<<11
#define LED_GOLIOTH_G 1<<12
#define LED_GOLIOTH_B 1<<9
#define LED_GOLIOTH LED_GOLIOTH_R
#define LED_USER_R 1<<7
#define LED_USER_G 1<<8
#define LED_USER_B 1<<6
#define LED_USER LED_USER_G
#define LED_ALL_MASK (LED_POWER | LED_BATTERY | LED_INTERNET | LED_GOLIOTH | LED_USER)

void led_init(void);
void led_push_mask(uint8_t mask);
void led_push_single(uint32_t led_bit, uint8_t state);

#endif
