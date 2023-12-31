#ifndef _LED_CTRL_H_
#define _LED_CTRL_H_

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
