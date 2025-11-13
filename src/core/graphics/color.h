#ifndef BOOSTIO_COLOR_H
#define BOOSTIO_COLOR_H

#include <stdint.h>

struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

static inline struct color color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	struct color c = {r, g, b, a};
	return c;
}

static inline struct color color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	return color_rgba(r, g, b, 255);
}

static inline struct color color_from_floats(float r, float g, float b, float a)
{
	return color_rgba(
		(uint8_t)(r * 255.0f),
		(uint8_t)(g * 255.0f),
		(uint8_t)(b * 255.0f),
		(uint8_t)(a * 255.0f)
	);
}

#define COLOR_BLACK color_rgb(0, 0, 0)
#define COLOR_WHITE color_rgb(255, 255, 255)
#define COLOR_RED color_rgb(255, 0, 0)
#define COLOR_GREEN color_rgb(0, 255, 0)
#define COLOR_BLUE color_rgb(0, 0, 255)
#define COLOR_YELLOW color_rgb(255, 255, 0)
#define COLOR_CYAN color_rgb(0, 255, 255)
#define COLOR_MAGENTA color_rgb(255, 0, 255)
#define COLOR_GRAY color_rgb(128, 128, 128)

#endif
