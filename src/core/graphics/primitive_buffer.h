#ifndef BOOSTIO_PRIMITIVE_BUFFER_H
#define BOOSTIO_PRIMITIVE_BUFFER_H

struct PrimitiveBuffer;
struct Color;

struct PrimitiveBuffer *primitive_buffer_create(void);
void primitive_buffer_destroy(struct PrimitiveBuffer *buffer);

void primitive_buffer_begin(struct PrimitiveBuffer *buffer);
void primitive_buffer_render(struct PrimitiveBuffer *buffer, int window_width, int window_height);

void primitive_buffer_add_rect(
	struct PrimitiveBuffer *buffer,
	float x,
	float y,
	float width,
	float height,
	struct Color color,
	float radius,
	float outline_width,
	struct Color outline_color
);
void primitive_buffer_add_line(
	struct PrimitiveBuffer *buffer, float x1, float y1, float x2, float y2, struct Color color
);

#endif
