#ifndef BOOSTIO_WINDOW_H
#define BOOSTIO_WINDOW_H

#include <GL/gl.h>
#include <SDL3/SDL.h>
#include <stdbool.h>

struct InputState
{
	bool scancodes_down[512];
	bool scancodes_pressed[512];
	bool scancodes_released[512];

	int mouse_x;
	int mouse_y;
	bool mouse_buttons[8];
	bool mouse_pressed[8];
	bool mouse_released[8];

	float mouse_wheel_x;
	float mouse_wheel_y;
};

struct WindowConfig
{
	int width;
	int height;
	const char *title;
	bool resizable;
	bool vsync;
};

struct Window
{
	SDL_Window *window;
	SDL_GLContext gl_context;
	int width;
	int height;
	struct InputState input;
	bool should_close;
};

struct Window *window_create(const struct WindowConfig *config);
void window_destroy(struct Window *window);
bool window_poll_events(struct Window *window);
void window_swap_buffers(struct Window *window);

void window_get_size(const struct Window *window, int *width, int *height);
void window_get_mouse_position(const struct Window *window, int *x, int *y);

bool window_is_scancode_down(const struct Window *window, SDL_Scancode scancode);
bool window_is_scancode_pressed(const struct Window *window, SDL_Scancode scancode);

bool window_is_mouse_button_down(const struct Window *window, int button);
bool window_is_mouse_button_pressed(const struct Window *window, int button);

#endif
