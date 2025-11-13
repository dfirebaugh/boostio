#ifndef BOOSTIO_LUA_SERVICE_H
#define BOOSTIO_LUA_SERVICE_H

#include <stdbool.h>

#include "core/lua/lua_api.h"
#include "core/lua/lua_runtime.h"
#include "core/platform/platform.h"

struct app_state;

struct lua_service
{
	struct lua_runtime runtime;
	struct lua_api_context api_context;
	bool initialized;
	char **loaded_plugins;
	int plugin_count;
	struct platform_paths *paths;
};

bool lua_service_init(
		struct lua_service *service, struct app_state *state, struct Graphics *graphics,
		struct platform_paths *paths
);

void lua_service_deinit(struct lua_service *service);

bool lua_service_load_config(struct lua_service *service, const char *config_path);

bool lua_service_load_plugins(struct lua_service *service);

bool lua_service_load_plugin(struct lua_service *service, const char *plugin_path);

void lua_service_call_render_callbacks(struct lua_service *service);

void lua_service_apply_config_to_state(struct lua_service *service, struct app_state *state);

#endif
