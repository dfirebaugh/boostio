#ifndef BOOSTIO_GRAPHICS_H
#define BOOSTIO_GRAPHICS_H

#include <stdbool.h>

struct window;
struct color;
struct graphics;
struct msdf_atlas;

struct graphics *graphics_create(struct window *window);
void graphics_destroy(struct graphics *graphics);

void graphics_clear(struct graphics *graphics, struct color color);
void graphics_set_color(struct graphics *graphics, struct color color);

void graphics_fill_rect(struct graphics *graphics, int x, int y, int width, int height);
void graphics_fill_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	struct color outline_color,
	int outline_width
);
void graphics_draw_rect(struct graphics *graphics, int x, int y, int width, int height);
void graphics_draw_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	struct color outline_color,
	int outline_width
);
void graphics_fill_rounded_rect(
	struct graphics *graphics, int x, int y, int width, int height, int radius
);
void graphics_fill_rounded_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	int radius,
	struct color outline_color,
	int outline_width
);
void graphics_draw_rounded_rect(
	struct graphics *graphics, int x, int y, int width, int height, int radius
);
void graphics_draw_rounded_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	int radius,
	struct color outline_color,
	int outline_width
);
void graphics_draw_line(struct graphics *graphics, int x1, int y1, int x2, int y2);

void graphics_present(struct graphics *graphics);

bool graphics_load_font(struct graphics *graphics, const char *json_path, const char *png_path);
void graphics_draw_text(struct graphics *graphics, const char *text, int x, int y, int size);
float graphics_measure_text(struct graphics *graphics, const char *text, int size);

bool graphics_should_close(const struct graphics *graphics);
bool graphics_poll_events(struct graphics *graphics);
struct window *graphics_get_window(struct graphics *graphics);
float graphics_get_fps(const struct graphics *graphics);

#endif
