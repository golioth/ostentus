#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ostentus_wrapper, LOG_LEVEL_DBG);

#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <string.h>

#define CMD_ADDR 0x12
#define DAT_ADDR 0x13

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
uint8_t _ostentus_data_buf[BUF_SIZE];

int send_ostentus_cmd(uint8_t cmd) {
	int ret = i2c_write(i2c_dev, &cmd, 1, CMD_ADDR);
	return ret;
}

int send_ostentus_data_buf(uint8_t len) {
	int ret = i2c_write(i2c_dev, _ostentus_data_buf, len, DAT_ADDR);
	return ret;
}

int clear_memory(void) {
	return send_ostentus_cmd(OSTENTUS_CLEAR_MEM);
}

int show_splash(void) {
	return send_ostentus_cmd(OSTENTUS_SPLASHSCREEN);
}

int update_display(void) {
	return send_ostentus_cmd(OSTENTUS_REFRESH);
}

int update_thickness(uint8_t thickness) {
	_ostentus_data_buf[0] = thickness;
	send_ostentus_data_buf(1);
	return send_ostentus_cmd(OSTENTUS_THICKNESS);
}

int update_font(uint8_t font) {
	_ostentus_data_buf[0] = font;
	send_ostentus_data_buf(1);
	return send_ostentus_cmd(OSTENTUS_FONT);
}

int clear_text_buffer(void) {
	return send_ostentus_cmd(OSTENTUS_CLEAR_TEXT);
}

int clear_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	_ostentus_data_buf[0] = x;
	_ostentus_data_buf[1] = y;
	_ostentus_data_buf[2] = w;
	_ostentus_data_buf[3] = h;
	send_ostentus_data_buf(4);
	return send_ostentus_cmd(OSTENTUS_CLEAR_RECT);
}

int led_bitmask(uint8_t bitmask) {
	_ostentus_data_buf[0] = bitmask;
	send_ostentus_data_buf(1);
	return send_ostentus_cmd(OSTENTUS_LED_BITMASK);
}

int store_text(char *str, uint8_t len) {
	memcpy(_ostentus_data_buf, str, len);
	send_ostentus_data_buf(len);
	return send_ostentus_cmd(OSTENTUS_STORE_TEXT);
	return 0;
}

int write_text(uint8_t x, uint8_t y, uint8_t thickness) {
	_ostentus_data_buf[0] = x;
	_ostentus_data_buf[1] = y;
	_ostentus_data_buf[2] = thickness;
	send_ostentus_data_buf(3);
	return send_ostentus_cmd(OSTENTUS_WRITE_TEXT);
}
