#include "ssd1681.hpp"

#include <cstdlib>
#include <math.h>

namespace pimoroni {

  enum reg {
    PSR      = 0x00,
    PWR      = 0x01,
    POF      = 0x02,
    PFS      = 0x03,
    PON      = 0x04,
    PMES     = 0x05,
    BTST     = 0x06,
    DSLP     = 0x07,
    DTM1     = 0x10,
    DSP      = 0x11,
    DRF      = 0x12,
    DTM2     = 0x13,
    LUT_VCOM = 0x20,
    LUT_WW   = 0x21,
    LUT_BW   = 0x22,
    LUT_WB   = 0x23,
    LUT_BB   = 0x24,
    PLL      = 0x30,
    TSC      = 0x40,
    TSE      = 0x41,
    TSR      = 0x43,
    TSW      = 0x42,
    CDI      = 0x50,
    LPD      = 0x51,
    TCON     = 0x60,
    TRES     = 0x61,
    REV      = 0x70,
    FLG      = 0x71,
    AMV      = 0x80,
    VV       = 0x81,
    VDCS     = 0x82,
    PTL      = 0x90,
    PTIN     = 0x91,
    PTOU     = 0x92,
    PGM      = 0xa0,
    APG      = 0xa1,
    ROTP     = 0xa2,
    CCSET    = 0xe0,
    PWS      = 0xe3,
    TSSET    = 0xe5
  };

  bool SSD1681::is_busy() {
    return gpio_get(BUSY);
  }

  void SSD1681::busy_wait() {
    return;
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
    spi_init(spi, 12'000'000);

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
    sleep_ms(100);

    command(0x01, {0xC7, 0x00, 0x00}); //Driver output control
    command(0x11, {0x01}); //data entry mode
    command(0x44, {0x00, 0x18}); //set Ram-X address start/end position
    command(0x45, {0xC7, 0x00, 0x00, 0x00}); //set Ram-Y address start/end position
    command(0x3C, {0x05}); //BorderWavefrom
    command(0x18, {0x80}); //Reading temperature sensor
    command(0x4E, {0x00});   // set RAM x address count to 0;
    command(0x4F, {0xC7, 0x00});   // set RAM y address count to 0X199;
    busy_wait();

//     _update_speed = speed;
// 
//     if(speed == 0) {
//       command(PSR, {
//         RES_128x296 | LUT_OTP | FORMAT_BW | SHIFT_RIGHT | BOOSTER_ON | RESET_NONE
//       });
//     } else {
//       command(PSR, {
//         RES_128x296 | LUT_REG | FORMAT_BW | SHIFT_RIGHT | BOOSTER_ON | RESET_NONE
//       });
//     }
//     switch(speed) {
//       case 0:
// Note: the defult luts are built in so we don't really need to flash them here
// they are preserved above for posterity and reference mostly.
//         break;
//       case 1:
//         medium_luts();
//         break;
//       case 2:
//         fast_luts();
//         break;
//       case 3:
//         turbo_luts();
//         break;
//       default:
//         break;
//     }
// 
//     command(PWR, {
//       VDS_INTERNAL | VDG_INTERNAL,
//       VCOM_VD | VGHL_16V,
//       0b101011,
//       0b101011,
//       0b101011
//     });
// 
//     command(PON); // power on
//     busy_wait();
// 
// booster soft start configuration
//     command(BTST, {
//       START_10MS | STRENGTH_3 | OFF_6_58US,
//       START_10MS | STRENGTH_3 | OFF_6_58US,
//       START_10MS | STRENGTH_3 | OFF_6_58US
//     });
// 
//     command(PFS, {
//       FRAMES_1
//     });
// 
//     command(TSE, {
//       TEMP_INTERNAL | OFFSET_0
//     });
// 
//     command(TCON, {0x22}); // tcon setting
//     command(CDI, {(uint8_t)(inverted ? 0b01'01'1100 : 0b01'00'1100)}); // vcom and data interval
// 
//     command(PLL, {
//       HZ_100
//     });
// 
//     command(POF);
//     busy_wait();
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
//     inverted = inv;
//     command(CDI, {(uint8_t)(inverted ? 0b01'01'1100 : 0b01'00'1100)}); // vcom and data interval
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

  void SSD1681::partial_update(int x, int y, int w, int h, bool blocking) {
    // y is given in columns ("banks"), which are groups of 8 horiontal pixels
    // x is given in pixels
//     if(blocking) {
//       busy_wait();
//     }
// 
//     int cols = h / 8;
//     int y1 = y / 8;
// int y2 = y1 + cols;
// 
//     int rows = w;
//     int x1 = x;
// int x2 = x + rows;
// 
//     uint8_t partial_window[7] = {
//       (uint8_t)(y),
//       (uint8_t)(y + h - 1),
//       (uint8_t)(x >> 8),
//       (uint8_t)(x & 0xff),
//       (uint8_t)((x + w - 1) >> 8),
//       (uint8_t)((x + w - 1) & 0xff),
//       0b00000001  // PT_SCAN
//     };
//     command(PON); // turn on
// 
//     command(PTIN); // enable partial mode
//     command(PTL, sizeof(partial_window), partial_window);
// 
//     command(DTM2);
//     for (auto dx = 0; dx < rows; dx++) {
//       int sx = dx + x1;
//       int sy = y1;
//       data(cols, &frame_buffer[sy + (sx * (height / 8))]);
//     }
//     command(DSP); // data stop
// 
//     command(DRF); // start display refresh
// 
//     if(blocking) {
//       busy_wait();
// 
//       command(POF); // turn off
//     }
  }

  void SSD1681::update(bool blocking) {
    if(blocking) {
      busy_wait();
    }
    command(0x24, (width * height) / 8, frame_buffer);
    command(0x22, {0xF7});
    command(0x20);
    busy_wait();
    command(0x22, {0x83});
    command(0x20);
    busy_wait();

//     command(PON); // turn on
// 
//     command(PTOU); // disable partial mode
// 
//     command(DTM2, (width * height) / 8, frame_buffer); // transmit framebuffer
//     command(DSP); // data stop
// 
//     command(DRF); // start display refresh

//     if(blocking) {
//       busy_wait();

//       command(POF); // turn off
//     }
  }

  void SSD1681::off() {
    busy_wait();
//     command(POF); // turn off
    command(0x10, {0x01}); //enter deep sleep
    sleep_ms(100);
  }

}
