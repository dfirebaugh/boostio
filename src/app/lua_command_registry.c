#include "lua_command_registry.h"
#include "core/lua/lua_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool lua_command_registry_init(struct lua_command_registry *registry, struct lua_runtime *runtime)
{
	if (registry == NULL || runtime == NULL)
	{
		return false;
	}

	memset(registry, 0, sizeof(struct lua_command_registry));
	registry->runtime = runtime;
	registry->command_capacity = 16;
	registry->keybinding_capacity = 64;

	registry->commands = calloc(registry->command_capacity, sizeof(struct lua_command_entry));
	if (registry->commands == NULL)
	{
		return false;
	}

	registry->keybindings =
			calloc(registry->keybinding_capacity, sizeof(struct lua_keybinding_entry));
	if (registry->keybindings == NULL)
	{
		free(registry->commands);
		return false;
	}

	return true;
}

void lua_command_registry_deinit(struct lua_command_registry *registry)
{
	if (registry == NULL)
	{
		return;
	}

	for (size_t i = 0; i < registry->command_count; i++)
	{
		free(registry->commands[i].command_name);
		lua_runtime_unref(registry->runtime, registry->commands[i].lua_callback_ref);
	}
	free(registry->commands);

	for (size_t i = 0; i < registry->keybinding_count; i++)
	{
		free(registry->keybindings[i].command_name);
	}
	free(registry->keybindings);

	memset(registry, 0, sizeof(struct lua_command_registry));
}

bool lua_command_registry_register_lua_command(
		struct lua_command_registry *registry, const char *command_name, int callback_ref
)
{
	if (registry == NULL || command_name == NULL)
	{
		return false;
	}

	for (size_t i = 0; i < registry->command_count; i++)
	{
		if (strcmp(registry->commands[i].command_name, command_name) == 0)
		{
			lua_runtime_unref(registry->runtime, registry->commands[i].lua_callback_ref);
			registry->commands[i].lua_callback_ref = callback_ref;
			return true;
		}
	}

	if (registry->command_count >= registry->command_capacity)
	{
		size_t new_capacity = registry->command_capacity * 2;
		struct lua_command_entry *new_commands =
				realloc(registry->commands, new_capacity * sizeof(struct lua_command_entry));
		if (new_commands == NULL)
		{
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
		struct lua_command_registry *registry, enum key key, bool shift, bool ctrl, bool alt,
		const char *command_name
)
{
	if (registry == NULL || command_name == NULL)
	{
		return false;
	}

	for (size_t i = 0; i < registry->keybinding_count; i++)
	{
		if (registry->keybindings[i].key == key && registry->keybindings[i].shift == shift &&
			registry->keybindings[i].ctrl == ctrl && registry->keybindings[i].alt == alt)
		{
			free(registry->keybindings[i].command_name);
			registry->keybindings[i].command_name = strdup(command_name);
			printf("Updated Lua keybinding: key=%d shift=%d ctrl=%d alt=%d -> %s\n", key, shift,
				   ctrl, alt, command_name);
			return true;
		}
	}

	if (registry->keybinding_count >= registry->keybinding_capacity)
	{
		size_t new_capacity = registry->keybinding_capacity * 2;
		struct lua_keybinding_entry *new_keybindings =
				realloc(registry->keybindings, new_capacity * sizeof(struct lua_keybinding_entry));
		if (new_keybindings == NULL)
		{
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

	printf("Registered Lua keybinding: key=%d shift=%d ctrl=%d alt=%d -> %s\n", key, shift, ctrl,
		   alt, command_name);
	return true;
}

bool lua_command_registry_execute_command(
		struct lua_command_registry *registry, const char *command_name
)
{
	if (registry == NULL || command_name == NULL)
	{
		return false;
	}

	for (size_t i = 0; i < registry->command_count; i++)
	{
		if (strcmp(registry->commands[i].command_name, command_name) == 0)
		{
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
	if (registry == NULL || key_event == NULL)
	{
		return NULL;
	}

	for (size_t i = 0; i < registry->keybinding_count; i++)
	{
		if (registry->keybindings[i].key == key_event->key &&
			registry->keybindings[i].shift == key_event->shift &&
			registry->keybindings[i].ctrl == key_event->ctrl &&
			registry->keybindings[i].alt == key_event->alt)
		{
			const char *command_name = registry->keybindings[i].command_name;

			for (size_t j = 0; j < registry->command_count; j++)
			{
				if (strcmp(registry->commands[j].command_name, command_name) == 0)
				{
					return command_name;
				}
			}

			fprintf(stderr, "Keybinding references unknown command: %s\n", command_name);
			return NULL;
		}
	}

	return NULL;
}
