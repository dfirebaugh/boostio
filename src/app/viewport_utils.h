#ifndef BOOSTIO_VIEWPORT_UTILS_H
#define BOOSTIO_VIEWPORT_UTILS_H

#include <stdint.h>

struct viewport;

void viewport_ms_to_x(const struct viewport *vp, uint32_t ms, float *out_x);

void viewport_x_to_ms(const struct viewport *vp, float x, uint32_t *out_ms);

void viewport_piano_key_to_y(const struct viewport *vp, uint8_t piano_key, float *out_y);

void viewport_y_to_piano_key(const struct viewport *vp, float y, uint8_t *out_key);

#endif
