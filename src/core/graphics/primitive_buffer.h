#ifndef BOOSTIO_PRIMITIVE_BUFFER_H
#define BOOSTIO_PRIMITIVE_BUFFER_H

struct primitive_buffer;
struct color;

struct primitive_buffer *primitive_buffer_create(void);
void primitive_buffer_destroy(struct primitive_buffer *buffer);

void primitive_buffer_begin(struct primitive_buffer *buffer);
void primitive_buffer_render(struct primitive_buffer *buffer, int window_width, int window_height);

void primitive_buffer_add_rect(
	struct primitive_buffer *buffer,
	float x,
	float y,
	float width,
	float height,
	struct color color,
	float radius,
	float outline_width,
	struct color outline_color
);
void primitive_buffer_add_line(
	struct primitive_buffer *buffer, float x1, float y1, float x2, float y2, struct color color
);

#endif
