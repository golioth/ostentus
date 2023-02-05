#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "i2c_multi.h"
#include "pico/stdlib.h"

#define OSTENTUS_ADDR 0x12

PIO pio = pio0;
uint pin = 4;
#define BUFFER_SIZE 64
uint8_t buffer[BUFFER_SIZE] = {0};

#define FIFO_SIZE 32
#define FIFO_DATA_OFFSET 2 // cmd addr, data_len
#define FIFO_DATA_SIZE BUFFER_SIZE-FIFO_DATA_OFFSET

typedef struct {
    uint8_t reg;
    uint8_t len;
    char data[BUFFER_SIZE-FIFO_DATA_OFFSET];
} i2c_msg_t;

typedef struct {
    uint8_t head;
    uint8_t tail;
    uint8_t has_data;
    uint8_t idx;
} fifo_status_t;

fifo_status_t _fs = {
    .head = 0,
    .tail = 0,
    .has_data = 0,
    .idx = 0
};

i2c_msg_t _fifo[FIFO_SIZE];

void i2c_receive_handler(uint8_t data, bool is_address)
{
    static bool previous_was_address = false;
    if ((_fs.head == _fs.tail) && _fs.has_data) {
        //Buffer is full, do nothing.
        return;
    }
    if (is_address) {
        previous_was_address = true;
    } else {
        if (previous_was_address) {
            _fifo[_fs.tail].reg = data;
            previous_was_address = false;
        } else {
            _fifo[_fs.tail].data[_fs.idx] = data;
            ++_fs.idx;
            if (_fs.idx >= FIFO_DATA_SIZE) {
                --_fs.idx;
            }
        }
    }
}

/* Currently unused, this is the example from the i2c_multi lib */
// https://github.com/dgatf/I2C-slave-multi-address-RP2040
void i2c_request_handler(uint8_t address)
{
    printf("\nAddress: %X, request...", address);

    switch (address)
    {
    case OSTENTUS_ADDR:
        buffer[0] = 0x10;
        buffer[1] = 0x11;
        buffer[2] = 0x12;
        break;
    }
}

void i2c_stop_handler(uint8_t length)
{
    //Write data len
    _fifo[_fs.tail].len = _fs.idx;
    //Indicate there's info in the fifo
    ++_fs.has_data;
    if (++_fs.tail >= FIFO_SIZE) {
        _fs.tail = 0;
    }

    //Reset to defaults
    _fs.idx = 0;
}

int fifo_pop(uint8_t * r_buf) {
    if (_fs.has_data == 0) {
        return -1;
    }
    r_buf[0] = _fifo[_fs.head].reg;
    r_buf[1] = _fifo[_fs.head].len;
    memcpy(r_buf+2, _fifo[_fs.head].data, _fifo[_fs.head].len);
    ++_fs.head;
    if (_fs.head >= FIFO_SIZE) {
        _fs.head = 0;
    }
    --_fs.has_data;
    return 0;
}

bool fifo_has_data(void) {
    if (_fs.has_data) {
        return true;
    }
    return false;
}

void fifo_init(void)
{
    _fs.head = 0;
    _fs.tail = 0;
    _fs.has_data = 0;
    _fs.idx = 0;

    i2c_multi_init(pio, pin);
    i2c_multi_enable_address(OSTENTUS_ADDR);
    i2c_multi_set_receive_handler(i2c_receive_handler);
    i2c_multi_set_request_handler(i2c_request_handler);
    i2c_multi_set_stop_handler(i2c_stop_handler);
    i2c_multi_set_write_buffer(buffer);
}
