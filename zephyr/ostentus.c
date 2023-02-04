#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ostentus_wrapper, LOG_LEVEL_DBG);

#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <string.h>

#define OSTENTUS_ADDR 0x12

#define OSTENTUS_CLEAR_MEM 0x00
#define OSTENTUS_REFRESH 0x01
#define OSTENTUS_ADDR_X 0x02
#define OSTENTUS_ADDR_Y 0x03
#define OSTENTUS_SPLASHSCREEN 0x04
#define OSTENTUS_THICKNESS 0x05
#define OSTENTUS_FONT 0x06
#define OSTENTUS_WRITE_TEXT 0x07
#define OSTENTUS_CLEAR_TEXT 0x08
#define OSTENTUS_CLEAR_RECT 0x09
#define OSTENTUS_LED_USE 0x10
#define OSTENTUS_LED_GOL 0x11
#define OSTENTUS_LED_INT 0x12
#define OSTENTUS_LED_BAT 0x13
#define OSTENTUS_LED_POW 0x14
#define OSTENTUS_LED_BITMASK 0x18
#define OSTENTUS_STRING_0 0x20
#define OSTENTUS_STRING_1 0x21
#define OSTENTUS_STRING_2 0x22
#define OSTENTUS_STRING_3 0x23
#define OSTENTUS_STRING_4 0x24
#define OSTENTUS_STRING_5 0x25
#define OSTENTUS_STORE_TEXT 0x26

const struct device *i2c_dev = DEVICE_DT_GET(DT_ALIAS(click_i2c));

#define BUF_SIZE 48
uint8_t _ostentus_buf[BUF_SIZE];

int ostentus_i2c_write(uint8_t reg, uint8_t data_len) {
	_ostentus_buf[0] = reg;
	LOG_HEXDUMP_DBG(_ostentus_buf, data_len+1, "sending packet");
	return i2c_write(i2c_dev, _ostentus_buf, data_len+1, OSTENTUS_ADDR);
}

int clear_memory(void) {
	return ostentus_i2c_write(OSTENTUS_CLEAR_MEM, 0);
}

int show_splash(void) {
	return ostentus_i2c_write(OSTENTUS_SPLASHSCREEN, 0);
}

int update_display(void) {
	return ostentus_i2c_write(OSTENTUS_REFRESH, 0);
}

int update_thickness(uint8_t thickness) {
	_ostentus_buf[1] = thickness;
	return ostentus_i2c_write(OSTENTUS_THICKNESS, 1);
}

int update_font(uint8_t font) {
	_ostentus_buf[1] = font;
	return ostentus_i2c_write(OSTENTUS_FONT, 1);
}

int clear_text_buffer(void) {
	return ostentus_i2c_write(OSTENTUS_CLEAR_TEXT, 0);
}

int clear_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	_ostentus_buf[1] = x;
	_ostentus_buf[2] = y;
	_ostentus_buf[3] = w;
	_ostentus_buf[4] = h;
	return ostentus_i2c_write(OSTENTUS_CLEAR_RECT, 4);
}

int led_bitmask(uint8_t bitmask) {
	_ostentus_buf[1] = bitmask;
	return ostentus_i2c_write(OSTENTUS_LED_BITMASK, 1);
}

int store_text(char *str, uint8_t len) {
	memcpy(_ostentus_buf+1, str, len);
	return ostentus_i2c_write(OSTENTUS_STORE_TEXT, len);
	return 0;
}

int write_text(uint8_t x, uint8_t y, uint8_t thickness) {
	_ostentus_buf[1] = x;
	_ostentus_buf[2] = y;
	_ostentus_buf[3] = thickness;
	return ostentus_i2c_write(OSTENTUS_WRITE_TEXT, 3);
}
