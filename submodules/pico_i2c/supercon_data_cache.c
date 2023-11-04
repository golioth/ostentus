#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../micropython/lib/pico-sdk/src/rp2_common/pico_rand/include/pico/rand.h"
#include "supercon_data_cache.h"
#include "supercon_data_transfer.h"

struct context {
    uint16_t point_idx;
} cache_ctx;

Samples _samples;
SuperconMeta _meta;

void finalize_samples(void) {
    stage_data(&_meta, &_samples, cache_ctx.point_idx);
    sample_cache_set_ready(true);
}

void init_samples(char *name, uint16_t interval_us) {
    cache_ctx.point_idx = 0;
    snprintf(_meta.header.name, sizeof(_meta.header.name), "%s", name);
    _meta.header.interval_us = interval_us;
    _meta.header.uid = (uint16_t)get_rand_32();
}

int put_point(Point p) {

    if (sample_cache_is_ready()) {
        /* Buffer is alreay staged for Alduel */
        return -1;
    }

    _samples.points[cache_ctx.point_idx] = p;

    if (cache_ctx.point_idx++ >= MAX_SAMPLES) {
        finalize_samples();
    }

    return 0;
}


