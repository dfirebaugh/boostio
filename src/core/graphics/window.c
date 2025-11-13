#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void reset_frame_input(struct InputState *input)
{
	memset(input->scancodes_pressed, 0, sizeof(input->scancodes_pressed));
	memset(input->scancodes_released, 0, sizeof(input->scancodes_released));
	memset(input->mouse_pressed, 0, sizeof(input->mouse_pressed));
	memset(input->mouse_released, 0, sizeof(input->mouse_released));
	input->mouse_wheel_x = 0.0f;
	input->mouse_wheel_y = 0.0f;
}

struct Window *window_create(const struct WindowConfig *config)
{
	if (!config)
	{
		fprintf(stderr, "Window config is NULL\n");
		return NULL;
	}

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return NULL;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	struct Window *window = (struct Window *)malloc(sizeof(struct Window));
	if (!window)
	{
		fprintf(stderr, "Failed to allocate window\n");
		return NULL;
	}

	memset(window, 0, sizeof(struct Window));

	window->width = config->width;
	window->height = config->height;
	window->should_close = false;

	Uint32 flags = SDL_WINDOW_OPENGL;
	if (config->resizable)
	{
		flags |= SDL_WINDOW_RESIZABLE;
	}

	window->window = SDL_CreateWindow(config->title, config->width, config->height, flags);

	if (!window->window)
	{
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		free(window);
		return NULL;
	}

	window->gl_context = SDL_GL_CreateContext(window->window);
	if (!window->gl_context)
	{
		fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(window->window);
		free(window);
		return NULL;
	}

	SDL_GL_SetSwapInterval(config->vsync ? 1 : 0);

	return window;
}

void window_destroy(struct Window *window)
{
	if (!window)
	{
		return;
	}

	if (window->gl_context)
	{
		SDL_GL_DestroyContext(window->gl_context);
	}

	if (window->window)
	{
		SDL_DestroyWindow(window->window);
	}

	free(window);
	SDL_Quit();
}

bool window_poll_events(struct Window *window)
{
	if (!window)
	{
		return false;
	}

	reset_frame_input(&window->input);

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			window->should_close = true;
			return false;

		case SDL_EVENT_KEY_DOWN:
		{
			SDL_Scancode scancode = event.key.scancode;
			if (scancode < 512)
			{
				if (!window->input.scancodes_down[scancode])
				{
					window->input.scancodes_pressed[scancode] = true;
				}
				window->input.scancodes_down[scancode] = true;
			}
			break;
		}

		case SDL_EVENT_KEY_UP:
		{
			SDL_Scancode scancode = event.key.scancode;
			if (scancode < 512)
			{
				window->input.scancodes_released[scancode] = true;
				window->input.scancodes_down[scancode] = false;
			}
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
			window->input.mouse_x = (int)event.motion.x;
			window->input.mouse_y = (int)event.motion.y;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			int button = event.button.button - 1;
			if (button >= 0 && button < 8)
			{
				if (!window->input.mouse_buttons[button])
				{
					window->input.mouse_pressed[button] = true;
				}
				window->input.mouse_buttons[button] = true;
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			int button = event.button.button - 1;
			if (button >= 0 && button < 8)
			{
				window->input.mouse_released[button] = true;
				window->input.mouse_buttons[button] = false;
			}
			break;
		}

		case SDL_EVENT_MOUSE_WHEEL:
			window->input.mouse_wheel_x = event.wheel.x;
			window->input.mouse_wheel_y = event.wheel.y;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
			window->width = event.window.data1;
			window->height = event.window.data2;
			break;
		}
	}

	return !window->should_close;
}

void window_swap_buffers(struct Window *window)
{
	if (!window || !window->window)
	{
		return;
	}
	SDL_GL_SwapWindow(window->window);
}

void window_get_size(const struct Window *window, int *width, int *height)
{
	if (window && width && height)
	{
		*width = window->width;
		*height = window->height;
	}
}

void window_get_mouse_position(const struct Window *window, int *x, int *y)
{
	if (window && x && y)
	{
		*x = window->input.mouse_x;
		*y = window->input.mouse_y;
	}
}

bool window_is_scancode_down(const struct Window *window, SDL_Scancode scancode)
{
	if (window && scancode < 512)
	{
		return window->input.scancodes_down[scancode];
	}
	return false;
}

bool window_is_scancode_pressed(const struct Window *window, SDL_Scancode scancode)
{
	if (window && scancode < 512)
	{
		return window->input.scancodes_pressed[scancode];
	}
	return false;
}

bool window_is_mouse_button_down(const struct Window *window, int button)
{
	if (window && button >= 0 && button < 8)
	{
		return window->input.mouse_buttons[button];
	}
	return false;
}

bool window_is_mouse_button_pressed(const struct Window *window, int button)
{
	if (window && button >= 0 && button < 8)
	{
		return window->input.mouse_pressed[button];
	}
	return false;
}
