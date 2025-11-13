#ifndef BOOSTIO_LUA_RUNTIME_H
#define BOOSTIO_LUA_RUNTIME_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>

struct lua_runtime
{
	lua_State *L;
};

bool lua_runtime_init(struct lua_runtime *runtime);

void lua_runtime_deinit(struct lua_runtime *runtime);

bool lua_runtime_load_file(struct lua_runtime *runtime, const char *filepath);

bool lua_runtime_load_string(struct lua_runtime *runtime, const char *code);

bool lua_runtime_call_function(struct lua_runtime *runtime, const char *function_name);

void lua_runtime_register_function(
		struct lua_runtime *runtime, const char *name, lua_CFunction func
);

const char *lua_runtime_get_config_string(
		struct lua_runtime *runtime, const char *path, const char *default_value
);

int lua_runtime_get_config_int(struct lua_runtime *runtime, const char *path, int default_value);

double
lua_runtime_get_config_number(struct lua_runtime *runtime, const char *path, double default_value);

bool lua_runtime_get_config_bool(struct lua_runtime *runtime, const char *path, bool default_value);

int lua_runtime_ref(struct lua_runtime *runtime, int stack_index);

void lua_runtime_unref(struct lua_runtime *runtime, int ref);

bool lua_runtime_call_ref(struct lua_runtime *runtime, int ref, int num_args);

void lua_runtime_add_package_path(struct lua_runtime *runtime, const char *path);

#endif
