#ifndef SUPERCON_DATA_TRANSFER_H
#define SUPERCON_DATA_TRANSFER_H

#include <stdint.h>
#include <stdbool.h>
#include "supercon_data_cache.h"

#define I2C_PACKET_SIZE 36

/* Sizing 36 bytes i2c packets:
 *
 * Header packet
 *    2 unique id
 *    2 blocknum (0x00, 0x00 indicates header packet)
 *    2 bytes interval between data points (in us)
 *    2 bytes total number of sample bytes (# of samples * 2 bytes)
 *    25 bytes for name (including null terminaor)
 *    3 bytes reserved for future use
 *
 * Data packets
 *    2 unique id
 *    2 blocknum (max == final block)
 *    32 bytes of data (16 stucts of 8-bit x and 8-bit y pairs)
 */

typedef struct {
    uint16_t uid;
    uint16_t block_id;
    uint16_t interval_us;
    uint16_t data_len;
    char name[25];
    uint8_t u8_reserved;
    uint16_t u16_reserved3;
    /* total 36 bytes */
} SuperconHeader;

union SuperconMeta_U {
    SuperconHeader header;
    uint8_t bytes[sizeof(SuperconHeader)];
} typedef SuperconMeta;

struct supercon_packet {
    uint16_t uid;
    uint16_t blocknum;
    Point data[16];
};

void stage_data(SuperconMeta *meta, Samples *data_array, uint16_t data_len);
bool sample_cache_is_ready(void);
void sample_cache_set_ready(bool ready);
bool data_available(void);
uint8_t get_data(void);

#endif
