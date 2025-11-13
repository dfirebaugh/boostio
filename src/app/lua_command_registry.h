#ifndef LUA_COMMAND_REGISTRY_H
#define LUA_COMMAND_REGISTRY_H

#include "core/input/input_types.h"

#include <stdbool.h>
#include <stddef.h>

struct lua_runtime;

struct lua_command_entry {
	char *command_name;
	int lua_callback_ref;
};

struct lua_keybinding_entry {
	enum key key;
	bool shift;
	bool ctrl;
	bool alt;
	char *command_name;
};

struct lua_command_registry {
	struct lua_runtime *runtime;
	struct lua_command_entry *commands;
	size_t command_count;
	size_t command_capacity;
	struct lua_keybinding_entry *keybindings;
	size_t keybinding_count;
	size_t keybinding_capacity;
};

bool lua_command_registry_init(struct lua_command_registry *registry, struct lua_runtime *runtime);
void lua_command_registry_deinit(struct lua_command_registry *registry);

bool lua_command_registry_register_lua_command(
	struct lua_command_registry *registry, const char *command_name, int callback_ref
);

bool lua_command_registry_register_keybinding(
	struct lua_command_registry *registry,
	enum key key,
	bool shift,
	bool ctrl,
	bool alt,
	const char *command_name
);

bool lua_command_registry_execute_command(
	struct lua_command_registry *registry, const char *command_name
);

const char *lua_command_registry_get_command_for_key(
	struct lua_command_registry *registry, struct input_event_key_down *key_event
);

const char *lua_command_registry_key_to_string(enum key key);

#endif
