#include "ssd1681.hpp"

#include <cstdlib>
#include <math.h>
#include <string.h>

namespace pimoroni {

  bool SSD1681::is_busy() {
    return gpio_get(BUSY);
  }

  void SSD1681::busy_wait() {
    while(is_busy()) {
      tight_loop_contents();
    }
  }

  void SSD1681::reset() {
    gpio_put(RESET, 0); sleep_ms(10);
    gpio_put(RESET, 1); sleep_ms(10);
    busy_wait();
  }

  void SSD1681::init() {
    // configure spi interface and pins
    spi_init(spi, 20'000'000);

    gpio_set_function(DC, GPIO_FUNC_SIO);
    gpio_set_dir(DC, GPIO_OUT);

    gpio_set_function(CS, GPIO_FUNC_SIO);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1);

    gpio_set_function(RESET, GPIO_FUNC_SIO);
    gpio_set_dir(RESET, GPIO_OUT);
    gpio_put(RESET, 1);

    gpio_set_function(BUSY, GPIO_FUNC_SIO);
    gpio_set_dir(BUSY, GPIO_IN);
    gpio_set_pulls(BUSY, false, true);

    gpio_set_function(SCK,  GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);

    setup();
  };

  void SSD1681::setup(uint8_t speed) {
    reset();

    busy_wait();
    command(0x12);  //SWRESET
    busy_wait();

    command(0x01, {0xC7, 0x00, 0x00}); //Driver output control
    command(0x11, {0x01}); //data entry mode
    command(0x44, {0x00, 0x18}); //set Ram-X address start/end position
    command(0x45, {0xC7, 0x00, 0x00, 0x00}); //set Ram-Y address start/end position
    command(0x3C, {0x05}); //BorderWavefrom
    command(0x18, {0x80}); //Reading temperature sensor
    command(0x4E, {0x00});   // set RAM x address count to 0;
    command(0x4F, {0xC7, 0x00});   // set RAM y address count to 0X199;
    busy_wait();
  }

  void SSD1681::power_off() {
    //command(POF);
  }

  void SSD1681::read(uint8_t reg, size_t len, uint8_t *data) {
    gpio_put(CS, 0);

    gpio_put(DC, 0); // command mode
    spi_write_blocking(spi, &reg, 1);

    if(len > 0) {
      gpio_put(DC, 1); // data mode
      gpio_set_function(SCK,  GPIO_FUNC_SIO);
      gpio_set_dir(SCK, GPIO_OUT);
      gpio_set_function(MOSI, GPIO_FUNC_SIO);
      gpio_set_dir(MOSI, GPIO_IN);
      for(auto i = 0u; i < len; i++) {
        int byte = i / 8;
        int bit = i % 8;
        gpio_put(SCK, true);
        bool value = gpio_get(MOSI);
        data[byte] |= value << (7-bit);
        gpio_put(SCK, false);
      }

      gpio_set_function(SCK,  GPIO_FUNC_SPI);
      gpio_set_function(MOSI, GPIO_FUNC_SPI);
    }

    gpio_put(CS, 1);
  }

  void SSD1681::command(uint8_t reg, size_t len, const uint8_t *data) {
    gpio_put(CS, 0);

    gpio_put(DC, 0); // command mode
    spi_write_blocking(spi, &reg, 1);

    if(len > 0) {
      gpio_put(DC, 1); // data mode
      spi_write_blocking(spi, (const uint8_t*)data, len);
    }

    gpio_put(CS, 1);
  }

  void SSD1681::data(size_t len, const uint8_t *data) {
    gpio_put(CS, 0);
    gpio_put(DC, 1); // data mode
    spi_write_blocking(spi, (const uint8_t*)data, len);
    gpio_put(CS, 1);
  }

  void SSD1681::command(uint8_t reg, std::initializer_list<uint8_t> values) {
    command(reg, values.size(), (uint8_t *)values.begin());
  }

  void SSD1681::pixel(int x, int y, int v) {
    // bounds check
    if(x < 0 || y < 0 || x >= width || y >= height) return;

    // pointer to byte in framebuffer that contains this pixel
    uint8_t *p = &frame_buffer[(y / 8) + (x * (height / 8))];

    uint8_t  o = 7 - (y & 0b111);       // bit offset within byte
    uint8_t  m = ~(1 << o);             // bit mask for byte
    uint8_t  b = (v == 0 ? 0 : 1) << o; // bit value shifted to position

    *p &= m; // clear bit
    *p |= b; // set bit value
  }

  uint8_t* SSD1681::get_frame_buffer() {
	  return frame_buffer;
  }

  void SSD1681::invert(bool inv) {
    inverted = inv;
    command(0x21, {(uint8_t)(inverted ? 0x08 : 0x00)}); // vcom and data interval
  }

  void SSD1681::update_speed(uint8_t speed) {
//     setup(speed);
  }

  uint8_t SSD1681::update_speed() {
    return _update_speed;
  }

  uint32_t SSD1681::update_time() {
    switch(_update_speed) {
      case 0:
        return 4500;
      case 1:
        return 2000;
      case 2:
        return 800;
      case 3:
        return 250;
      default:
        return 4500;
    }
  }

  void SSD1681::partial_update_data(int x, int y, int w, int h, bool blocking) {
    if(blocking) {
      busy_wait();
    }
    reset();

    int cols = h / 8;
    int y1 = y / 8;

    int rows = w;
    //Mirror the X axis
    int x1 = width - x - 1;

    uint8_t partial_window[6] = {
      (uint8_t)y1,
      (uint8_t)(y1 + cols - 1),
      (uint8_t)(x1 & 0xff),
      (uint8_t)(x1 >> 8),
      (uint8_t)((x1 + w - 1) >> 8),
      (uint8_t)((x1 + w - 1) & 0xff)
    };

    //Lock the border to prevent flashing
    command(0x3C, {0x80}); //BorderWavefrom,

    // set RAM x address start/end, in page 35
    // RAM x address start at 00h;
    // RAM x address end at 0fh(15+1)*8->128 
    command(0x44, 2, partial_window);

    // set RAM y address start/end, in page 35
    // RAM y address start at 0127h;
    // RAM y address start at 0127h;
    // RAM y address end at 00h;
    // ????=0
    command(0x45, 4, partial_window+2);

    command(0x4E, {partial_window[0]}); // set RAM x address count to 0;
    command(0x4F, 2, partial_window+2); // set RAM y address count to 0X127;

    command(0x24);   //Write Black and White image to RAM
    for (auto dx = 0; dx < rows; dx++) {
      int sx = dx + x;
      int sy = y1;
      data(cols, &frame_buffer[sy + (sx * (height / 8))]);
    }
  }

  void SSD1681::partial_update_execute() {
    busy_wait();
    command(0x22, {0xFF});
    command(0x20);
    busy_wait();
  }

  void SSD1681::partial_update(int x, int y, int w, int h, bool blocking) {
    partial_update_data(x, y, w, h, blocking);
    partial_update_execute();
  }

  void SSD1681::update(bool blocking) {
    if(blocking) {
      busy_wait();
    }

    setup();

    command(0x24, (width * height) / 8, frame_buffer);
    command(0x26, (width * height) / 8, frame_buffer);
    command(0x22, {0xF7});
    command(0x20);
    busy_wait();
  }

  void SSD1681::off() {
    busy_wait();
    command(0x10, {0x01}); //enter deep sleep
    sleep_ms(100);
  }
}
