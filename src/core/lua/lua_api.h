#ifndef BOOSTIO_LUA_API_H
#define BOOSTIO_LUA_API_H

#include "core/lua/lua_runtime.h"

struct Graphics;

struct lua_api_context
{
	struct Graphics *graphics;
};

void lua_api_register_all(struct lua_runtime *runtime, struct lua_api_context *ctx);

void lua_api_set_context(struct lua_api_context *ctx);

#endif
