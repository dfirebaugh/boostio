#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdbool.h>
#include <stddef.h>

#include "input_types.h"

struct window;

struct input_handler {
	struct window *window;
	float last_mouse_x;
	float last_mouse_y;
	bool shift_held;
	bool ctrl_held;
	bool alt_held;
};

struct input_handler *input_handler_create(struct window *window);
void input_handler_destroy(struct input_handler *handler);

size_t input_handler_poll_events(
	struct input_handler *handler, struct input_event *events, size_t max_events
);

#endif
