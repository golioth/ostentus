#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "supercon_data_cache.h"
#include "supercon_data_transfer.h"

typedef struct {
    bool ready;
    SuperconMeta *meta;
    Samples *data_array;
    uint16_t data_len;
    uint16_t packet_idx;
    uint16_t data_idx;
    bool is_last_packet;
} TransferCtx;

TransferCtx _cached = {
    .ready = false,
    .meta = NULL,
    .data_array = NULL,
    .data_len = 0,
    .packet_idx = 0,
    .data_idx = 0,
    .is_last_packet = false,
};

void stage_data(SuperconMeta *meta, Samples *data_array, uint16_t data_len) {
    meta->header.data_len = data_len;
    _cached.meta = meta;
    _cached.data_array = data_array;
    _cached.data_len = data_len;
    _cached.is_last_packet = false;
}

bool data_available(void) {
    return _cached.ready;
}

static uint8_t get_next_header_byte(void) {
    return _cached.meta->bytes[_cached.packet_idx];
}

bool is_last_packet(void) {
    if (_cached.packet_idx < I2C_PACKET_SIZE) return false;

    /* we're indexing bytes, but data_len is ints */
    uint16_t data_bytes_per_packet = I2C_PACKET_SIZE - 4;
    uint16_t bytes_remaining = (_cached.data_len * 2) - _cached.data_idx;
    return bytes_remaining <= data_bytes_per_packet;
}

static uint8_t get_next_data_byte(void) {
    if (_cached.data_idx >= _cached.data_len * 2) {
        return 0;
    } else {
        return _cached.data_array->bytes[_cached.data_idx++];
    }
}

uint8_t get_byte_from_int(uint16_t starting_int, uint8_t send_upper_byte) {
    if (send_upper_byte) {
        return (uint8_t)(starting_int >> 8);
    } else {
        return (uint8_t)starting_int;
    }
}

static uint8_t get_next_in_packet(void) {
    uint16_t mod_idx = _cached.packet_idx % I2C_PACKET_SIZE;
    uint8_t return_data = 0;

    if (mod_idx < 2) {
        /* UID */
        return_data = get_byte_from_int(_cached.meta->header.uid, mod_idx);
    } else if (mod_idx < 4) {
        /* Block number */
        if (is_last_packet()) {
            /* This is the final block, use 0xFFFF for the block number */
            return_data = 0xFF;
        } else {
            return_data = get_byte_from_int(_cached.packet_idx / I2C_PACKET_SIZE, mod_idx % 2);
        }
    } else if (_cached.packet_idx < I2C_PACKET_SIZE) {
        /* Currently sending header */
        return_data = get_next_header_byte();
    } else {
        /* Currently sending data */
        return_data = get_next_data_byte();
    }

    /* Handle increment and checks for end of data */
    _cached.packet_idx++;

    if (_cached.data_idx >= _cached.data_len * 2) {
        /* Set last packet when we run out of data */
        _cached.is_last_packet = true;
    }

    if (_cached.is_last_packet && _cached.packet_idx % I2C_PACKET_SIZE == 0) {
        /* This was the last byte of the last packet */
        _cached.ready = false;
    }

    return return_data;
}

uint8_t get_data(void) {
    if (!_cached.ready) return -1;
    if (_cached.meta == NULL) return -2;
    if (_cached.data_array == NULL) return -3;
    if (_cached.data_len == 0) return -4;

    return get_next_in_packet();
}

bool sample_cache_is_ready(void) {
    return _cached.ready;
}

void sample_cache_set_ready(bool ready) {
    _cached.ready = ready;
    if (ready) {
        _cached.packet_idx = 0;
        _cached.data_idx = 0;
    }
}
