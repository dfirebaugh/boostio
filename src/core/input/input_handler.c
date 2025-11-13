#include "input_handler.h"
#include "input_mapper.h"
#include "window.h"

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

struct input_handler *input_handler_create(struct window *window)
{
	if (window == NULL) {
		return NULL;
	}

	struct input_handler *handler = malloc(sizeof(struct input_handler));
	if (handler == NULL) {
		return NULL;
	}

	memset(handler, 0, sizeof(struct input_handler));
	handler->window = window;

	return handler;
}

void input_handler_destroy(struct input_handler *handler)
{
	if (handler == NULL) {
		return;
	}

	free(handler);
}

static bool is_modifier_key(enum key key)
{
	return key == KEY_LEFT_SHIFT || key == KEY_RIGHT_SHIFT || key == KEY_LEFT_CONTROL ||
	       key == KEY_RIGHT_CONTROL || key == KEY_LEFT_ALT || key == KEY_RIGHT_ALT;
}

size_t input_handler_poll_events(
	struct input_handler *handler, struct input_event *events, size_t max_events
)
{
	if (handler == NULL || events == NULL || max_events == 0) {
		return 0;
	}

	size_t event_count = 0;
	struct window *window = handler->window;

	handler->shift_held = window_is_scancode_down(window, SDL_SCANCODE_LSHIFT) ||
			      window_is_scancode_down(window, SDL_SCANCODE_RSHIFT);
	handler->ctrl_held = window_is_scancode_down(window, SDL_SCANCODE_LCTRL) ||
			     window_is_scancode_down(window, SDL_SCANCODE_RCTRL);
	handler->alt_held = window_is_scancode_down(window, SDL_SCANCODE_LALT) ||
			    window_is_scancode_down(window, SDL_SCANCODE_RALT);

	for (int scancode = 0; scancode < 512 && event_count < max_events; scancode++) {
		if (window_is_scancode_pressed(window, scancode)) {
			enum key key;
			if (input_mapper_scancode_to_key(scancode, &key)) {
				if (is_modifier_key(key)) {
					continue;
				}

				events[event_count].type = INPUT_EVENT_KEY_DOWN;
				events[event_count].data.key_down.key = key;
				events[event_count].data.key_down.shift = handler->shift_held;
				events[event_count].data.key_down.ctrl = handler->ctrl_held;
				events[event_count].data.key_down.alt = handler->alt_held;
				event_count++;
			}
		}
	}

	for (int scancode = 0; scancode < 512 && event_count < max_events; scancode++) {
		if (window->input.scancodes_released[scancode]) {
			enum key key;
			if (input_mapper_scancode_to_key(scancode, &key)) {
				if (is_modifier_key(key)) {
					continue;
				}

				events[event_count].type = INPUT_EVENT_KEY_UP;
				events[event_count].data.key_up.key = key;
				event_count++;
			}
		}
	}

	for (int button = 0; button < 3 && event_count < max_events; button++) {
		if (window_is_mouse_button_pressed(window, button)) {
			int mouse_x, mouse_y;
			window_get_mouse_position(window, &mouse_x, &mouse_y);

			events[event_count].type = INPUT_EVENT_MOUSE_DOWN;
			events[event_count].data.mouse_down.x = (float)mouse_x;
			events[event_count].data.mouse_down.y = (float)mouse_y;
			events[event_count].data.mouse_down.button = (enum mouse_button)button;
			events[event_count].data.mouse_down.shift = handler->shift_held;
			events[event_count].data.mouse_down.ctrl = handler->ctrl_held;
			events[event_count].data.mouse_down.alt = handler->alt_held;
			event_count++;
		}
	}

	for (int button = 0; button < 3 && event_count < max_events; button++) {
		if (window->input.mouse_released[button]) {
			int mouse_x, mouse_y;
			window_get_mouse_position(window, &mouse_x, &mouse_y);

			events[event_count].type = INPUT_EVENT_MOUSE_UP;
			events[event_count].data.mouse_up.x = (float)mouse_x;
			events[event_count].data.mouse_up.y = (float)mouse_y;
			events[event_count].data.mouse_up.button = (enum mouse_button)button;
			event_count++;
		}
	}

	int mouse_x, mouse_y;
	window_get_mouse_position(window, &mouse_x, &mouse_y);
	if (mouse_x != handler->last_mouse_x || mouse_y != handler->last_mouse_y) {
		if (event_count < max_events) {
			events[event_count].type = INPUT_EVENT_MOUSE_MOVE;
			events[event_count].data.mouse_move.x = (float)mouse_x;
			events[event_count].data.mouse_move.y = (float)mouse_y;
			event_count++;
		}
		handler->last_mouse_x = (float)mouse_x;
		handler->last_mouse_y = (float)mouse_y;
	}

	if ((window->input.mouse_wheel_x != 0.0f || window->input.mouse_wheel_y != 0.0f) &&
	    event_count < max_events) {
		events[event_count].type = INPUT_EVENT_SCROLL;
		events[event_count].data.scroll.dx = window->input.mouse_wheel_x;
		events[event_count].data.scroll.dy = window->input.mouse_wheel_y;
		events[event_count].data.scroll.shift = handler->shift_held;
		events[event_count].data.scroll.ctrl = handler->ctrl_held;
		events[event_count].data.scroll.alt = handler->alt_held;
		event_count++;
	}

	return event_count;
}
