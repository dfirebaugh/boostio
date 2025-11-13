#include "app/app_controller.h"
#include "core/graphics/color.h"
#include "core/graphics/window.h"

#include <stdbool.h>
#include <stdio.h>

bool app_controller_init(
		struct app_controller *controller, struct Graphics *graphics, struct Audio *audio, struct platform_paths *paths
)
{
	if (controller == NULL || graphics == NULL || audio == NULL)
	{
		return false;
	}

	controller->graphics = graphics;
	controller->audio = audio;
	controller->paths = paths;
	controller->running = true;

	app_state_init(&controller->state);

	struct Window *window = graphics_get_window(graphics);
	controller->input_handler = input_handler_create(window);
	if (controller->input_handler == NULL)
	{
		fprintf(stderr, "Failed to create input handler\n");
		return false;
	}

	controller->test_button.text = "click me";
	controller->test_button.x = 350;
	controller->test_button.y = 350;
	controller->test_button.width = 80;
	controller->test_button.height = 50;

	return true;
}

void app_controller_deinit(struct app_controller *controller)
{
	if (controller == NULL)
	{
		return;
	}

	lua_service_deinit(&controller->lua_service);
	input_handler_destroy(controller->input_handler);
}

bool app_controller_init_lua(struct app_controller *controller, const char *config_path)
{
	if (controller == NULL || config_path == NULL)
	{
		return false;
	}

	if (!lua_service_init(
				&controller->lua_service, &controller->state, controller->graphics,
				controller->audio, controller->paths
		))
	{
		fprintf(stderr, "Failed to initialize Lua service\n");
		return false;
	}

	if (!lua_service_load_config(&controller->lua_service, config_path))
	{
		fprintf(stderr, "Failed to load Lua config\n");
		return false;
	}

	lua_service_apply_config_to_state(&controller->lua_service, &controller->state);

	lua_service_set_app_controller(&controller->lua_service, controller);

	if (!lua_service_load_plugins(&controller->lua_service))
	{
		fprintf(stderr, "Warning: Failed to load some plugins\n");
	}

	return true;
}

static void handle_input_event(struct app_controller *controller, struct input_event *event)
{
	if (event == NULL)
	{
		return;
	}

	const char *command_name =
			lua_service_get_command_for_event(&controller->lua_service, event);

	if (command_name != NULL)
	{
		lua_service_execute_lua_command(&controller->lua_service, command_name);
	}
}

void app_controller_update(struct app_controller *controller, float delta_time)
{
	if (controller == NULL)
	{
		return;
	}

	if (graphics_should_close(controller->graphics))
	{
		controller->running = false;
	}

	struct input_event events[64];
	size_t event_count = input_handler_poll_events(controller->input_handler, events, 64);

	for (size_t i = 0; i < event_count; i++)
	{
		handle_input_event(controller, &events[i]);
	}
}

void app_controller_render(struct app_controller *controller)
{
	if (controller == NULL || controller->graphics == NULL)
	{
		return;
	}

	struct Color bg_color = color_rgb(30, 30, 46);
	graphics_clear(controller->graphics, bg_color);

	/* test button */
	button_render(controller->graphics, &controller->test_button);
	lua_service_call_render_callbacks(&controller->lua_service);
}

bool app_controller_is_running(const struct app_controller *controller)
{
	if (controller == NULL)
	{
		return false;
	}

	return controller->running;
}

void app_controller_stop(struct app_controller *controller)
{
	if (controller == NULL)
	{
		return;
	}

	controller->running = false;
}
