#include "app/app_controller.h"
#include "core/audio/audio.h"
#include "core/audio/sequencer.h"
#include "core/graphics/color.h"
#include "core/graphics/graphics.h"
#include "core/graphics/window.h"
#include "core/input/input_handler.h"

#include <stdbool.h>
#include <stdio.h>

bool app_controller_init(
	struct app_controller *controller,
	struct graphics *graphics,
	struct audio *audio,
	struct platform_paths *paths
)
{
	if (controller == NULL || graphics == NULL || audio == NULL) {
		return false;
	}

	controller->graphics = graphics;
	controller->audio = audio;
	controller->paths = paths;
	controller->running = true;

	app_state_init(&controller->state);

	struct window *window = graphics_get_window(graphics);
	controller->input_handler = input_handler_create(window);
	if (controller->input_handler == NULL) {
		fprintf(stderr, "Failed to create input handler\n");
		return false;
	}

	return true;
}

void app_controller_deinit(struct app_controller *controller)
{
	if (controller == NULL) {
		return;
	}

	lua_service_deinit(&controller->lua_service);
	input_handler_destroy(controller->input_handler);
}

bool app_controller_init_lua(struct app_controller *controller, const char *config_path)
{
	if (controller == NULL || config_path == NULL) {
		return false;
	}

	if (!lua_service_init(
		    &controller->lua_service,
		    &controller->state,
		    controller->graphics,
		    controller->audio,
		    controller->paths
	    )) {
		fprintf(stderr, "Failed to initialize Lua service\n");
		return false;
	}

	if (!lua_service_load_config(&controller->lua_service, config_path)) {
		fprintf(stderr, "Failed to load Lua config\n");
		return false;
	}

	lua_service_apply_config_to_state(&controller->lua_service, &controller->state);

	lua_service_set_app_controller(&controller->lua_service, controller);

	if (!lua_service_load_plugins(&controller->lua_service)) {
		fprintf(stderr, "Warning: Failed to load some plugins\n");
	}

	return true;
}

static void handle_input_event(struct app_controller *controller, struct input_event *event)
{
	if (event == NULL) {
		return;
	}

	if (event->type == INPUT_EVENT_KEY_DOWN) {
		bool handled = lua_service_dispatch_key_event(
			&controller->lua_service, &event->data.key_down
		);
		if (handled) {
			return;
		}
	}

	const char *command_name =
		lua_service_get_command_for_event(&controller->lua_service, event);

	if (command_name != NULL) {
		lua_service_execute_lua_command(&controller->lua_service, command_name);
	}
}

void app_controller_update(struct app_controller *controller, float delta_time)
{
	if (controller == NULL) {
		return;
	}

	if (graphics_should_close(controller->graphics)) {
		controller->running = false;
	}

	struct window *window = graphics_get_window(controller->graphics);
	int current_width, current_height;
	window_get_size(window, &current_width, &current_height);

	if (current_width != controller->state.window_width ||
	    current_height != controller->state.window_height) {
		app_state_update_dimensions(&controller->state, current_width, current_height);
	}

	if (window->input.mouse_wheel_y != 0.0f) {
		bool shift_down = window_is_scancode_down(window, SDL_SCANCODE_LSHIFT) ||
				  window_is_scancode_down(window, SDL_SCANCODE_RSHIFT);
		bool ctrl_down = window_is_scancode_down(window, SDL_SCANCODE_LCTRL) ||
				 window_is_scancode_down(window, SDL_SCANCODE_RCTRL);

		if (ctrl_down && shift_down) {
			float zoom_factor = 1.0f + (window->input.mouse_wheel_y * 0.1f);
			app_state_zoom_horizontal(&controller->state, zoom_factor);
		} else if (shift_down) {
			app_state_scroll_horizontal(
				&controller->state, -window->input.mouse_wheel_y * 100.0f
			);
		} else if (ctrl_down) {
			float zoom_factor = 1.0f + (window->input.mouse_wheel_y * 0.1f);
			app_state_zoom_vertical(&controller->state, zoom_factor);
		} else {
			app_state_scroll_vertical(
				&controller->state, -(int)window->input.mouse_wheel_y * 3
			);
		}
	}

	if (window->input.mouse_wheel_x != 0.0f) {
		if (window_is_scancode_down(window, SDL_SCANCODE_LCTRL) ||
		    window_is_scancode_down(window, SDL_SCANCODE_RCTRL)) {
			float zoom_factor = 1.0f + (window->input.mouse_wheel_x * 0.1f);
			app_state_zoom_horizontal(&controller->state, zoom_factor);
		} else {
			app_state_scroll_horizontal(
				&controller->state, -window->input.mouse_wheel_x * 100.0f
			);
		}
	}

	if (window_is_scancode_down(window, SDL_SCANCODE_LEFT)) {
		app_state_scroll_horizontal(&controller->state, -500.0f * delta_time);
	}
	if (window_is_scancode_down(window, SDL_SCANCODE_RIGHT)) {
		app_state_scroll_horizontal(&controller->state, 500.0f * delta_time);
	}
	if (window_is_scancode_down(window, SDL_SCANCODE_UP)) {
		app_state_scroll_vertical(&controller->state, -1);
	}
	if (window_is_scancode_down(window, SDL_SCANCODE_DOWN)) {
		app_state_scroll_vertical(&controller->state, 1);
	}

	struct sequencer *sequencer = audio_get_sequencer(controller->audio);
	if (sequencer) {
		controller->state.playing = sequencer->playing;
		if (sequencer->sample_rate > 0) {
			controller->state.playhead_ms = (uint32_t)((sequencer->playhead_samples * 1000) / sequencer->sample_rate);
		}
	}

	struct input_event events[64];
	size_t event_count = input_handler_poll_events(controller->input_handler, events, 64);

	for (size_t i = 0; i < event_count; i++) {
		handle_input_event(controller, &events[i]);
	}
}

void app_controller_render(struct app_controller *controller)
{
	if (controller == NULL || controller->graphics == NULL) {
		return;
	}

	struct color bg_color = color_rgb(30, 30, 46);
	graphics_clear(controller->graphics, bg_color);

	lua_service_call_render_callbacks(&controller->lua_service);
}

bool app_controller_is_running(const struct app_controller *controller)
{
	if (controller == NULL) {
		return false;
	}

	return controller->running;
}

void app_controller_stop(struct app_controller *controller)
{
	if (controller == NULL) {
		return;
	}

	controller->running = false;
}
