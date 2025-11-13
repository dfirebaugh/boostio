#ifndef BOOSTIO_LUA_SERVICE_H
#define BOOSTIO_LUA_SERVICE_H

#include <stdbool.h>

#include "core/lua/lua_api.h"
#include "core/lua/lua_runtime.h"
#include "lua_command_registry.h"

struct app_state;
struct command;
struct input_event;
struct platform_paths;

struct lua_service {
	struct lua_runtime runtime;
	struct lua_api_context api_context;
	struct lua_command_registry command_registry;
	bool initialized;
	char **loaded_plugins;
	int plugin_count;
	struct platform_paths *paths;
};

bool lua_service_init(
	struct lua_service *service,
	struct app_state *state,
	struct graphics *graphics,
	struct audio *audio,
	struct platform_paths *paths
);

void lua_service_deinit(struct lua_service *service);

bool lua_service_load_config(struct lua_service *service, const char *config_path);

bool lua_service_load_plugins(struct lua_service *service);

bool lua_service_load_plugin(struct lua_service *service, const char *plugin_path);

void lua_service_call_render_callbacks(struct lua_service *service);

bool lua_service_dispatch_key_event(
	struct lua_service *service, struct input_event_key_down *event
);

void lua_service_apply_config_to_state(struct lua_service *service, struct app_state *state);

const char *
lua_service_get_command_for_event(struct lua_service *service, struct input_event *event);

bool lua_service_execute_lua_command(struct lua_service *service, const char *command_name);

void lua_service_set_app_controller(struct lua_service *service, struct app_controller *controller);

#endif
