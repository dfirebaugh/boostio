#include <stdbool.h>
#include <stdio.h>

#include "app/app_controller.h"
#include "core/graphics/color.h"


bool app_controller_init(struct app_controller *controller, struct Graphics *graphics,
						 struct platform_paths *paths)
{
	if (controller == NULL || graphics == NULL)
	{
		return false;
	}

	controller->graphics = graphics;
	controller->paths = paths;
	controller->running = true;

	app_state_init(&controller->state);

	return true;
}

void app_controller_deinit(struct app_controller *controller)
{
	if (controller == NULL)
	{
		return;
	}

	lua_service_deinit(&controller->lua_service);
}

bool app_controller_init_lua(struct app_controller *controller, const char *config_path)
{
	if (controller == NULL || config_path == NULL)
	{
		return false;
	}

	if (!lua_service_init(&controller->lua_service, &controller->state, controller->graphics,
						  controller->paths))
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

	if (!lua_service_load_plugins(&controller->lua_service))
	{
		fprintf(stderr, "Warning: Failed to load some plugins\n");
	}

	return true;
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
}

void app_controller_render(struct app_controller *controller)
{
	if (controller == NULL || controller->graphics == NULL)
	{
		return;
	}

	struct Color bg_color = color_rgb(30, 30, 46);
	graphics_clear(controller->graphics, bg_color);

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
