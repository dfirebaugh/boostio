#ifndef BOOSTIO_APP_CONTROLLER_H
#define BOOSTIO_APP_CONTROLLER_H

#include "app/app_state.h"
#include "app/lua_service.h"
#include "app/ui_components/button.h"
#include "core/graphics/graphics.h"
#include "core/platform/platform.h"
#include "core/input/input_handler.h"

#include <stdbool.h>

struct app_controller
{
	struct app_state state;
	struct lua_service lua_service;
	struct Graphics *graphics;
	struct platform_paths *paths;
	struct input_handler *input_handler;
	bool running;

	struct button test_button;
};

bool app_controller_init(
		struct app_controller *controller, struct Graphics *graphics, struct platform_paths *paths
);

void app_controller_deinit(struct app_controller *controller);

bool app_controller_init_lua(struct app_controller *controller, const char *config_path);

void app_controller_update(struct app_controller *controller, float delta_time);

void app_controller_render(struct app_controller *controller);

bool app_controller_is_running(const struct app_controller *controller);

void app_controller_stop(struct app_controller *controller);

#endif
