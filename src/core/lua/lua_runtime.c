#include "core/lua/lua_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void push_config_path(lua_State *L, const char *path)
{
	lua_getglobal(L, "config");

	char *path_copy = strdup(path);
	char *token = strtok(path_copy, ".");

	while (token != NULL) {
		if (lua_type(L, -1) != LUA_TTABLE) {
			lua_pop(L, 1);
			free(path_copy);
			lua_pushnil(L);
			return;
		}

		lua_getfield(L, -1, token);
		lua_remove(L, -2);
		token = strtok(NULL, ".");
	}

	free(path_copy);
}

bool lua_runtime_init(struct lua_runtime *runtime)
{
	if (runtime == NULL) {
		return false;
	}

	runtime->L = luaL_newstate();
	if (runtime->L == NULL) {
		fprintf(stderr, "Failed to create Lua state\n");
		return false;
	}

	luaL_openlibs(runtime->L);
	return true;
}

void lua_runtime_deinit(struct lua_runtime *runtime)
{
	if (runtime == NULL || runtime->L == NULL) {
		return;
	}

	lua_close(runtime->L);
	runtime->L = NULL;
}

bool lua_runtime_load_file(struct lua_runtime *runtime, const char *filepath)
{
	if (runtime == NULL || runtime->L == NULL || filepath == NULL) {
		return false;
	}

	if (luaL_loadfile(runtime->L, filepath) != LUA_OK) {
		fprintf(stderr, "Lua load error: %s\n", lua_tostring(runtime->L, -1));
		lua_pop(runtime->L, 1);
		return false;
	}

	if (lua_pcall(runtime->L, 0, LUA_MULTRET, 0) != LUA_OK) {
		fprintf(stderr, "Lua execution error: %s\n", lua_tostring(runtime->L, -1));
		lua_pop(runtime->L, 1);
		return false;
	}

	return true;
}

bool lua_runtime_load_string(struct lua_runtime *runtime, const char *code)
{
	if (runtime == NULL || runtime->L == NULL || code == NULL) {
		return false;
	}

	if (luaL_loadstring(runtime->L, code) != LUA_OK) {
		fprintf(stderr, "Lua load error: %s\n", lua_tostring(runtime->L, -1));
		lua_pop(runtime->L, 1);
		return false;
	}

	if (lua_pcall(runtime->L, 0, LUA_MULTRET, 0) != LUA_OK) {
		fprintf(stderr, "Lua execution error: %s\n", lua_tostring(runtime->L, -1));
		lua_pop(runtime->L, 1);
		return false;
	}

	return true;
}

bool lua_runtime_call_function(struct lua_runtime *runtime, const char *function_name)
{
	if (runtime == NULL || runtime->L == NULL || function_name == NULL) {
		return false;
	}

	lua_getglobal(runtime->L, function_name);
	if (!lua_isfunction(runtime->L, -1)) {
		lua_pop(runtime->L, 1);
		return false;
	}

	if (lua_pcall(runtime->L, 0, 0, 0) != LUA_OK) {
		fprintf(stderr, "Lua function error: %s\n", lua_tostring(runtime->L, -1));
		lua_pop(runtime->L, 1);
		return false;
	}

	return true;
}

void lua_runtime_register_function(
	struct lua_runtime *runtime, const char *name, lua_CFunction func
)
{
	if (runtime == NULL || runtime->L == NULL || name == NULL || func == NULL) {
		return;
	}

	lua_pushcfunction(runtime->L, func);
	lua_setglobal(runtime->L, name);
}

const char *lua_runtime_get_config_string(
	struct lua_runtime *runtime, const char *path, const char *default_value
)
{
	if (runtime == NULL || runtime->L == NULL || path == NULL) {
		return default_value;
	}

	push_config_path(runtime->L, path);

	if (lua_type(runtime->L, -1) != LUA_TSTRING) {
		lua_pop(runtime->L, 1);
		return default_value;
	}

	const char *value = lua_tostring(runtime->L, -1);
	lua_pop(runtime->L, 1);
	return value;
}

int lua_runtime_get_config_int(struct lua_runtime *runtime, const char *path, int default_value)
{
	if (runtime == NULL || runtime->L == NULL || path == NULL) {
		return default_value;
	}

	push_config_path(runtime->L, path);

	if (lua_type(runtime->L, -1) != LUA_TNUMBER) {
		lua_pop(runtime->L, 1);
		return default_value;
	}

	int value = (int)lua_tointeger(runtime->L, -1);
	lua_pop(runtime->L, 1);
	return value;
}

double
lua_runtime_get_config_number(struct lua_runtime *runtime, const char *path, double default_value)
{
	if (runtime == NULL || runtime->L == NULL || path == NULL) {
		return default_value;
	}

	push_config_path(runtime->L, path);

	if (lua_type(runtime->L, -1) != LUA_TNUMBER) {
		lua_pop(runtime->L, 1);
		return default_value;
	}

	double value = lua_tonumber(runtime->L, -1);
	lua_pop(runtime->L, 1);
	return value;
}

bool lua_runtime_get_config_bool(struct lua_runtime *runtime, const char *path, bool default_value)
{
	if (runtime == NULL || runtime->L == NULL || path == NULL) {
		return default_value;
	}

	push_config_path(runtime->L, path);

	if (lua_type(runtime->L, -1) != LUA_TBOOLEAN) {
		lua_pop(runtime->L, 1);
		return default_value;
	}

	bool value = lua_toboolean(runtime->L, -1);
	lua_pop(runtime->L, 1);
	return value;
}

int lua_runtime_ref(struct lua_runtime *runtime, int stack_index)
{
	if (runtime == NULL || runtime->L == NULL) {
		return LUA_REFNIL;
	}

	lua_pushvalue(runtime->L, stack_index);
	return luaL_ref(runtime->L, LUA_REGISTRYINDEX);
}

void lua_runtime_unref(struct lua_runtime *runtime, int ref)
{
	if (runtime == NULL || runtime->L == NULL) {
		return;
	}

	luaL_unref(runtime->L, LUA_REGISTRYINDEX, ref);
}

bool lua_runtime_call_ref(struct lua_runtime *runtime, int ref, int num_args)
{
	if (runtime == NULL || runtime->L == NULL) {
		return false;
	}

	lua_rawgeti(runtime->L, LUA_REGISTRYINDEX, ref);
	if (!lua_isfunction(runtime->L, -1)) {
		lua_pop(runtime->L, 1 + num_args);
		return false;
	}

	if (num_args > 0) {
		lua_insert(runtime->L, -(num_args + 1));
	}

	if (lua_pcall(runtime->L, num_args, 0, 0) != LUA_OK) {
		fprintf(stderr, "Lua callback error: %s\n", lua_tostring(runtime->L, -1));
		lua_pop(runtime->L, 1);
		return false;
	}

	return true;
}

void lua_runtime_add_package_path(struct lua_runtime *runtime, const char *path)
{
	if (runtime == NULL || runtime->L == NULL || path == NULL) {
		return;
	}

	lua_State *L = runtime->L;

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");

	const char *current_path = lua_tostring(L, -1);

	char new_path[2048];
	snprintf(new_path, sizeof(new_path), "%s/?.lua;%s", path, current_path);

	lua_pop(L, 1);
	lua_pushstring(L, new_path);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
}
