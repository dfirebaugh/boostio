#ifndef BOOSTIO_GRAPHICS_H
#define BOOSTIO_GRAPHICS_H

#include <stdbool.h>

struct Window;
struct Color;
struct Graphics;
struct MSDFAtlas;

struct Graphics *graphics_create(struct Window *window);
void graphics_destroy(struct Graphics *graphics);

void graphics_clear(struct Graphics *graphics, struct Color color);
void graphics_set_color(struct Graphics *graphics, struct Color color);

void graphics_fill_rect(struct Graphics *graphics, int x, int y, int width, int height);
void graphics_fill_rect_outlined(
		struct Graphics *graphics, int x, int y, int width, int height, struct Color outline_color,
		int outline_width
);
void graphics_draw_rect(struct Graphics *graphics, int x, int y, int width, int height);
void graphics_draw_rect_outlined(
		struct Graphics *graphics, int x, int y, int width, int height, struct Color outline_color,
		int outline_width
);
void graphics_fill_rounded_rect(
		struct Graphics *graphics, int x, int y, int width, int height, int radius
);
void graphics_fill_rounded_rect_outlined(
		struct Graphics *graphics, int x, int y, int width, int height, int radius,
		struct Color outline_color, int outline_width
);
void graphics_draw_rounded_rect(
		struct Graphics *graphics, int x, int y, int width, int height, int radius
);
void graphics_draw_rounded_rect_outlined(
		struct Graphics *graphics, int x, int y, int width, int height, int radius,
		struct Color outline_color, int outline_width
);
void graphics_draw_line(struct Graphics *graphics, int x1, int y1, int x2, int y2);

void graphics_present(struct Graphics *graphics);

bool graphics_load_font(struct Graphics *graphics, const char *json_path, const char *png_path);
void graphics_draw_text(struct Graphics *graphics, const char *text, int x, int y, int size);
float graphics_measure_text(struct Graphics *graphics, const char *text, int size);

bool graphics_should_close(const struct Graphics *graphics);
bool graphics_poll_events(struct Graphics *graphics);

#endif
