#ifndef SUPERCON_DATA_CACHE_H
#define SUPERCON_DATA_CACHE_H

#include <stdint.h>

//FIXME: This should be dynamically allocated
#define MAX_SAMPLES 32768 // Samples are 2 bytes each

typedef struct {
    uint8_t x;
    uint8_t y;
} Point;

typedef union {
    Point points[MAX_SAMPLES];
    uint8_t bytes[MAX_SAMPLES * 2];
} Samples;

void finalize_samples(void);
void init_samples(char *name, uint16_t interval_us);
int put_point(uint8_t x, uint8_t y);

#endif
