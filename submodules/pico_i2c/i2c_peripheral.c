#include <stdio.h>
#include <tusb.h>
#include "pico/stdlib.h"
#include "i2c_fifo.h"
#include "dummy_data.h"

int main() {
    stdio_init_all();
    setup_default_uart();
    fifo_init();

    while (!tud_cdc_connected()) { sleep_ms(100);  }

    printf("\nStarting Pico i2c responder\n");

    uint8_t i2c_buf[I2C_BUFFER_SIZE];

    /* fake some captured samples */
    generate_data(432);

    while(1) {
        /*
        while(!fifo_has_data()) sleep_ms(200);

        fifo_pop(i2c_buf);

        for (uint8_t i = 0; i < i2c_buf[1] + 2; i++) {
            printf("0x%02x ", i2c_buf[i]);
        }
        printf("\n");
        */
        sleep_ms(2000);
    }
}
