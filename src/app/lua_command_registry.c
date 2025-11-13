#include "lua_command_registry.h"
#include "core/lua/lua_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool lua_command_registry_init(struct lua_command_registry *registry, struct lua_runtime *runtime)
{
	if (registry == NULL || runtime == NULL) {
		return false;
	}

	memset(registry, 0, sizeof(struct lua_command_registry));
	registry->runtime = runtime;
	registry->command_capacity = 16;
	registry->keybinding_capacity = 64;

	registry->commands = calloc(registry->command_capacity, sizeof(struct lua_command_entry));
	if (registry->commands == NULL) {
		return false;
	}

	registry->keybindings =
		calloc(registry->keybinding_capacity, sizeof(struct lua_keybinding_entry));
	if (registry->keybindings == NULL) {
		free(registry->commands);
		return false;
	}

	return true;
}

void lua_command_registry_deinit(struct lua_command_registry *registry)
{
	if (registry == NULL) {
		return;
	}

	for (size_t i = 0; i < registry->command_count; i++) {
		free(registry->commands[i].command_name);
		lua_runtime_unref(registry->runtime, registry->commands[i].lua_callback_ref);
	}
	free(registry->commands);

	for (size_t i = 0; i < registry->keybinding_count; i++) {
		free(registry->keybindings[i].command_name);
	}
	free(registry->keybindings);

	memset(registry, 0, sizeof(struct lua_command_registry));
}

bool lua_command_registry_register_lua_command(
	struct lua_command_registry *registry, const char *command_name, int callback_ref
)
{
	if (registry == NULL || command_name == NULL) {
		return false;
	}

	for (size_t i = 0; i < registry->command_count; i++) {
		if (strcmp(registry->commands[i].command_name, command_name) == 0) {
			lua_runtime_unref(
				registry->runtime, registry->commands[i].lua_callback_ref
			);
			registry->commands[i].lua_callback_ref = callback_ref;
			return true;
		}
	}

	if (registry->command_count >= registry->command_capacity) {
		size_t new_capacity = registry->command_capacity * 2;
		struct lua_command_entry *new_commands =
			realloc(registry->commands,
				new_capacity * sizeof(struct lua_command_entry));
		if (new_commands == NULL) {
			return false;
		}
		registry->commands = new_commands;
		registry->command_capacity = new_capacity;
	}

	registry->commands[registry->command_count].command_name = strdup(command_name);
	registry->commands[registry->command_count].lua_callback_ref = callback_ref;
	registry->command_count++;

	return true;
}

bool lua_command_registry_register_keybinding(
	struct lua_command_registry *registry,
	enum key key,
	bool shift,
	bool ctrl,
	bool alt,
	const char *command_name
)
{
	if (registry == NULL || command_name == NULL) {
		return false;
	}

	for (size_t i = 0; i < registry->keybinding_count; i++) {
		if (registry->keybindings[i].key == key &&
		    registry->keybindings[i].shift == shift &&
		    registry->keybindings[i].ctrl == ctrl && registry->keybindings[i].alt == alt) {
			free(registry->keybindings[i].command_name);
			registry->keybindings[i].command_name = strdup(command_name);
			printf("Updated Lua keybinding: key=%d shift=%d ctrl=%d alt=%d -> %s\n",
			       key,
			       shift,
			       ctrl,
			       alt,
			       command_name);
			return true;
		}
	}

	if (registry->keybinding_count >= registry->keybinding_capacity) {
		size_t new_capacity = registry->keybinding_capacity * 2;
		struct lua_keybinding_entry *new_keybindings =
			realloc(registry->keybindings,
				new_capacity * sizeof(struct lua_keybinding_entry));
		if (new_keybindings == NULL) {
			return false;
		}
		registry->keybindings = new_keybindings;
		registry->keybinding_capacity = new_capacity;
	}

	registry->keybindings[registry->keybinding_count].key = key;
	registry->keybindings[registry->keybinding_count].shift = shift;
	registry->keybindings[registry->keybinding_count].ctrl = ctrl;
	registry->keybindings[registry->keybinding_count].alt = alt;
	registry->keybindings[registry->keybinding_count].command_name = strdup(command_name);
	registry->keybinding_count++;

	printf("Registered Lua keybinding: key=%d shift=%d ctrl=%d alt=%d -> %s\n",
	       key,
	       shift,
	       ctrl,
	       alt,
	       command_name);
	return true;
}

bool lua_command_registry_execute_command(
	struct lua_command_registry *registry, const char *command_name
)
{
	if (registry == NULL || command_name == NULL) {
		return false;
	}

	for (size_t i = 0; i < registry->command_count; i++) {
		if (strcmp(registry->commands[i].command_name, command_name) == 0) {
			return lua_runtime_call_ref(
				registry->runtime, registry->commands[i].lua_callback_ref, 0
			);
		}
	}

	fprintf(stderr, "Command not found: %s\n", command_name);
	return false;
}

const char *lua_command_registry_get_command_for_key(
	struct lua_command_registry *registry, struct input_event_key_down *key_event
)
{
	if (registry == NULL || key_event == NULL) {
		return NULL;
	}

	for (size_t i = 0; i < registry->keybinding_count; i++) {
		if (registry->keybindings[i].key == key_event->key &&
		    registry->keybindings[i].shift == key_event->shift &&
		    registry->keybindings[i].ctrl == key_event->ctrl &&
		    registry->keybindings[i].alt == key_event->alt) {
			const char *command_name = registry->keybindings[i].command_name;

			for (size_t j = 0; j < registry->command_count; j++) {
				if (strcmp(registry->commands[j].command_name, command_name) == 0) {
					return command_name;
				}
			}

			fprintf(stderr,
				"Keybinding references unknown command: %s\n",
				command_name);
			return NULL;
		}
	}

	return NULL;
}

const char *lua_command_registry_key_to_string(enum key key)
{
	switch (key) {
	case KEY_0:
		return "0";
	case KEY_1:
		return "1";
	case KEY_2:
		return "2";
	case KEY_3:
		return "3";
	case KEY_4:
		return "4";
	case KEY_5:
		return "5";
	case KEY_6:
		return "6";
	case KEY_7:
		return "7";
	case KEY_8:
		return "8";
	case KEY_9:
		return "9";
	case KEY_A:
		return "a";
	case KEY_B:
		return "b";
	case KEY_C:
		return "c";
	case KEY_D:
		return "d";
	case KEY_E:
		return "e";
	case KEY_F:
		return "f";
	case KEY_G:
		return "g";
	case KEY_H:
		return "h";
	case KEY_I:
		return "i";
	case KEY_J:
		return "j";
	case KEY_K:
		return "k";
	case KEY_L:
		return "l";
	case KEY_M:
		return "m";
	case KEY_N:
		return "n";
	case KEY_O:
		return "o";
	case KEY_P:
		return "p";
	case KEY_Q:
		return "q";
	case KEY_R:
		return "r";
	case KEY_S:
		return "s";
	case KEY_T:
		return "t";
	case KEY_U:
		return "u";
	case KEY_V:
		return "v";
	case KEY_W:
		return "w";
	case KEY_X:
		return "x";
	case KEY_Y:
		return "y";
	case KEY_Z:
		return "z";
	case KEY_SPACE:
		return "space";
	case KEY_TAB:
		return "tab";
	case KEY_ESCAPE:
		return "escape";
	case KEY_DELETE:
		return "delete";
	case KEY_ENTER:
		return "enter";
	case KEY_BACKSPACE:
		return "backspace";
	case KEY_UP:
		return "up";
	case KEY_DOWN:
		return "down";
	case KEY_LEFT:
		return "left";
	case KEY_RIGHT:
		return "right";
	case KEY_F1:
		return "f1";
	case KEY_F2:
		return "f2";
	case KEY_F3:
		return "f3";
	case KEY_F4:
		return "f4";
	case KEY_F5:
		return "f5";
	case KEY_F6:
		return "f6";
	case KEY_F7:
		return "f7";
	case KEY_F8:
		return "f8";
	case KEY_F9:
		return "f9";
	case KEY_F10:
		return "f10";
	case KEY_F11:
		return "f11";
	case KEY_F12:
		return "f12";
	case KEY_MINUS:
		return "minus";
	case KEY_EQUALS:
		return "equals";
	case KEY_LEFT_SHIFT:
	case KEY_RIGHT_SHIFT:
		return "shift";
	case KEY_LEFT_CONTROL:
	case KEY_RIGHT_CONTROL:
		return "ctrl";
	case KEY_LEFT_ALT:
	case KEY_RIGHT_ALT:
		return "alt";
	default:
		return NULL;
	}
}
