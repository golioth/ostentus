#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>

#include "i2c_fifo.h"
#include "led_ctrl.h"
#include <libostentus_regmap.h>

uint8_t _semver[3] = { 1, 2, 3 };

#define OSTENTUS_ADDR 0x12
#define OSTENTUS_I2C_PORT i2c0
#define OSTENTUS_SDA_PIN 4
#define OSTENTUS_SCL_PIN 5
#define OSTENTUS_BAUDRATE 100000 // 100 kHz

uint8_t buffer[I2C_BUFFER_SIZE] = {0};

#define FIFO_SIZE 64
#define FIFO_DATA_OFFSET 2 // cmd addr, data_len
#define FIFO_DATA_SIZE I2C_BUFFER_SIZE-FIFO_DATA_OFFSET

typedef struct {
    uint8_t reg;
    uint8_t len;
    char data[I2C_BUFFER_SIZE-FIFO_DATA_OFFSET];
} i2c_msg_t;

typedef enum {
    I2C_IDLE,
    I2C_RECEIVE,
    I2C_HAS_READ_ADDR,
    I2C_REQUEST_PENDING
} fifo_state_t;

typedef struct {
    fifo_state_t state;

    uint8_t head;
    uint8_t tail;
    uint8_t has_data;
    uint8_t idx;

    i2c_msg_t fifo[FIFO_SIZE];
} fifo_status_t;

fifo_status_t ctx = {
    .state = I2C_IDLE,
    .head = 0,
    .tail = 0,
    .has_data = 0,
    .idx = 0
};

static void reset_ctx(void) {
    ctx.state = I2C_IDLE;
    ctx.head = 0;
    ctx.tail = 0;
    ctx.has_data = 0;
    ctx.idx = 0;
}

static i2c_msg_t *get_head(void) {
    return &ctx.fifo[ctx.head];
}

static i2c_msg_t *get_tail(void) {
    return &ctx.fifo[ctx.tail];
}

static void store_byte(uint8_t data) {
    if ((ctx.head == ctx.tail) && ctx.has_data) {
        //Buffer is full, do nothing.
        return;
    }
    if (ctx.state == I2C_IDLE) {
        get_tail()->reg = data;
        switch(data) {
            /** All "read" registers must be listed here
             *
             * These come from libostentus_regmap.h and are used to differentiate between register
             * addresses received before the controller requests to read bytes and a register
             * address received as a command without any additional data bytes.
             **/
            case OSTENTUS_GET_VERSION:
            case OSTENTUS_ISREADY:
            case OSTENTUS_RESET:
                ctx.state = I2C_HAS_READ_ADDR;
                break;
            default:
                ctx.state = I2C_RECEIVE;
        }
    } else {
        get_tail()->data[ctx.idx] = data;

        ++ctx.idx;
        if (ctx.idx >= FIFO_DATA_SIZE) {
            ctx.idx = FIFO_DATA_OFFSET - 1;
        }
    }
}

static uint8_t read_byte(void) {
    switch(get_tail()->reg) {
        case OSTENTUS_GET_VERSION:
            if (ctx.idx < sizeof(_semver)) {
                return _semver[ctx.idx++];
            } else {
                return 0xff;
            }
            break;
        default:
            return 0xff;
    }
}

static void process_immediate_or_enqueue(void)
{
    i2c_msg_t *buf = get_tail();
    // Write data len
    buf->len = ctx.idx;

    switch(buf->reg) {
        // Handle LED change directly (don't save to fifo)
        case ADDR_POWER:
            if (buf->len) {
                led_push_single(LED_POWER, buf->data[0]);
            }
            break;
        case ADDR_BATTERY:
            if (buf->len) {
                led_push_single(LED_BATTERY, buf->data[0]);
            }
            break;
        case ADDR_INTERNET:
            if (buf->len) {
                led_push_single(LED_INTERNET, buf->data[0]);
            }
            break;
        case ADDR_GOLIOTH:
            if (buf->len) {
                led_push_single(LED_GOLIOTH, buf->data[0]);
            }
            break;
        case ADDR_USER:
            if (buf->len) {
                led_push_single(LED_USER, buf->data[0]);
            }
            break;
        case ADDR_BITMASK:
            if (buf->len) {
                led_push_mask(buf->data[0]);
            }
            break;

        // Everything else goes into the fifo
        default:
            // Indicate there's info in the fifo
            if (++ctx.has_data > FIFO_SIZE) {
                ctx.has_data = FIFO_SIZE;
            };
            if (++ctx.tail >= FIFO_SIZE) {
                ctx.tail = 0;
            }
    }
}

static void stop_byte(void)
{
    switch(ctx.state) {
        case I2C_RECEIVE:
            process_immediate_or_enqueue();
            ctx.state = I2C_IDLE;
            ctx.idx = 0;
            break;

        case I2C_HAS_READ_ADDR:
            /* we just received the reg address, update state (do not reset!) */
            ctx.state = I2C_REQUEST_PENDING;
            return;

        case I2C_REQUEST_PENDING:
            /* Stop after read, time to reset */
            ctx.state = I2C_IDLE;
            ctx.idx = 0;
            break;

        default:
            ctx.state = I2C_IDLE;
            ctx.idx = 0;
    }
}

static void i2c_responder_handler(i2c_inst_t *i2c, i2c_slave_event_t event)
{
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        store_byte(i2c_read_byte_raw(i2c));
        break;

    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory
        i2c_write_byte_raw(i2c, read_byte());
        break;

    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        stop_byte();
        break;

    default:
        break;
    }
}

int fifo_pop(uint8_t * r_buf) {
    if (ctx.has_data == 0) {
        return -1;
    }
    r_buf[0] = get_head()->reg;
    r_buf[1] = get_head()->len;
    memcpy(r_buf+2, get_head()->data, get_head()->len);
    ++ctx.head;
    if (ctx.head >= FIFO_SIZE) {
        ctx.head = 0;
    }
    --ctx.has_data;
    return 0;
}

bool fifo_has_data(void) {
    if (ctx.has_data) {
        return true;
    }
    return false;
}

void fifo_init(void)
{
    reset_ctx();

    led_init();

    gpio_init(OSTENTUS_SDA_PIN);
    gpio_set_function(OSTENTUS_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(OSTENTUS_SDA_PIN);

    gpio_init(OSTENTUS_SCL_PIN);
    gpio_set_function(OSTENTUS_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(OSTENTUS_SCL_PIN);

    i2c_init(OSTENTUS_I2C_PORT, OSTENTUS_BAUDRATE);
    // configure I2C for slave mode
    i2c_slave_init(OSTENTUS_I2C_PORT, OSTENTUS_ADDR, &i2c_responder_handler);
}
