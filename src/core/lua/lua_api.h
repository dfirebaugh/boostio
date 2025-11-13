#ifndef BOOSTIO_LUA_API_H
#define BOOSTIO_LUA_API_H

#include "core/lua/lua_runtime.h"

struct graphics;
struct audio;
struct lua_command_registry;
struct app_state;
struct app_controller;

struct lua_api_context {
	struct graphics *graphics;
	struct audio *audio;
	struct lua_command_registry *command_registry;
	struct app_state *app_state;
	struct app_controller *app_controller;
};

void lua_api_register_all(struct lua_runtime *runtime, struct lua_api_context *ctx);

void lua_api_set_context(struct lua_api_context *ctx);

#endif
