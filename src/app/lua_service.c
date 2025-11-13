#include "app/lua_service.h"
#include "app/app_state.h"
#include "core/input/input_types.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool lua_service_init(
		struct lua_service *service, struct app_state *state, struct Graphics *graphics,
		struct platform_paths *paths
)
{
	if (service == NULL)
	{
		return false;
	}

	service->initialized = false;
	service->loaded_plugins = NULL;
	service->plugin_count = 0;
	service->paths = paths;

	if (!lua_runtime_init(&service->runtime))
	{
		fprintf(stderr, "Failed to initialize Lua runtime\n");
		return false;
	}

	if (!lua_command_registry_init(&service->command_registry, &service->runtime))
	{
		fprintf(stderr, "Failed to initialize Lua command registry\n");
		lua_runtime_deinit(&service->runtime);
		return false;
	}

	service->api_context.graphics = graphics;
	service->api_context.command_registry = &service->command_registry;
	service->api_context.app_state = state;

	lua_api_register_all(&service->runtime, &service->api_context);

	service->initialized = true;
	return true;
}

void lua_service_deinit(struct lua_service *service)
{
	if (service == NULL || !service->initialized)
	{
		return;
	}

	if (service->loaded_plugins != NULL)
	{
		for (int i = 0; i < service->plugin_count; i++)
		{
			free(service->loaded_plugins[i]);
		}
		free(service->loaded_plugins);
		service->loaded_plugins = NULL;
	}

	lua_command_registry_deinit(&service->command_registry);
	lua_runtime_deinit(&service->runtime);
	service->initialized = false;
}

bool lua_service_load_config(struct lua_service *service, const char *config_path)
{
	if (service == NULL || !service->initialized || config_path == NULL)
	{
		return false;
	}

	const char *last_slash = strrchr(config_path, '/');
	if (last_slash != NULL)
	{
		size_t dir_len = last_slash - config_path;
		char config_dir[512];
		snprintf(config_dir, sizeof(config_dir), "%.*s", (int)dir_len, config_path);
		lua_runtime_add_package_path(&service->runtime, config_dir);
	}

	if (!lua_runtime_load_file(&service->runtime, config_path))
	{
		fprintf(stderr, "Failed to load config from %s\n", config_path);
		return false;
	}

	printf("Loaded config from %s\n", config_path);
	return true;
}

bool lua_service_load_plugin(struct lua_service *service, const char *plugin_path)
{
	if (service == NULL || !service->initialized || plugin_path == NULL)
	{
		return false;
	}

	lua_State *L = service->runtime.L;

	if (luaL_loadfile(L, plugin_path) != LUA_OK)
	{
		fprintf(stderr, "Failed to load plugin %s: %s\n", plugin_path, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	if (lua_pcall(L, 0, 1, 0) != LUA_OK)
	{
		fprintf(stderr, "Failed to execute plugin %s: %s\n", plugin_path, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	const char *plugin_name = strrchr(plugin_path, '/');
	if (plugin_name == NULL)
	{
		plugin_name = plugin_path;
	}
	else
	{
		plugin_name++;
	}

	char name_buffer[256];
	strncpy(name_buffer, plugin_name, sizeof(name_buffer) - 1);
	name_buffer[sizeof(name_buffer) - 1] = '\0';

	char *dot = strrchr(name_buffer, '.');
	if (dot != NULL)
	{
		*dot = '\0';
	}

	lua_setglobal(L, name_buffer);

	printf("Loaded plugin: %s (as %s)\n", plugin_path, name_buffer);
	return true;
}

bool lua_service_load_plugins(struct lua_service *service)
{
	if (service == NULL || !service->initialized)
	{
		return false;
	}

	lua_State *L = service->runtime.L;
	if (L == NULL)
	{
		return false;
	}

	lua_getglobal(L, "config");
	if (lua_type(L, -1) != LUA_TTABLE)
	{
		fprintf(stderr, "Config not found or not a table\n");
		lua_pop(L, 1);
		return false;
	}

	lua_getfield(L, -1, "plugins");
	if (lua_type(L, -1) != LUA_TTABLE)
	{
		fprintf(stderr, "config.plugins not found\n");
		lua_pop(L, 2);
		return false;
	}

	lua_getfield(L, -1, "load_list");
	if (lua_type(L, -1) != LUA_TTABLE)
	{
		fprintf(stderr, "config.plugins.load_list not found\n");
		lua_pop(L, 3);
		return false;
	}

	int list_length = lua_rawlen(L, -1);

	service->loaded_plugins = malloc(sizeof(char *) * list_length);
	if (service->loaded_plugins == NULL)
	{
		lua_pop(L, 3);
		return false;
	}

	for (int i = 1; i <= list_length; i++)
	{
		lua_rawgeti(L, -1, i);
		if (lua_type(L, -1) != LUA_TTABLE)
		{
			lua_pop(L, 1);
			continue;
		}

		lua_getfield(L, -1, "enabled");
		bool enabled = lua_toboolean(L, -1);
		lua_pop(L, 1);

		if (!enabled)
		{
			lua_pop(L, 1);
			continue;
		}

		lua_getfield(L, -1, "name");
		const char *plugin_name = lua_tostring(L, -1);

		if (plugin_name != NULL)
		{
			char plugin_path[512];
			snprintf(
					plugin_path, sizeof(plugin_path), "%s/plugins/%s.lua", service->paths->data_dir,
					plugin_name
			);

			if (lua_service_load_plugin(service, plugin_path))
			{
				service->loaded_plugins[service->plugin_count] = strdup(plugin_name);
				service->plugin_count++;
			}
		}

		lua_pop(L, 2);
	}

	lua_pop(L, 3);

	printf("Loaded %d plugins from config\n", service->plugin_count);
	return true;
}

void lua_service_call_render_callbacks(struct lua_service *service)
{
	if (service == NULL || !service->initialized)
	{
		return;
	}

	lua_State *L = service->runtime.L;
	if (L == NULL)
	{
		return;
	}

	for (int i = 0; i < service->plugin_count; i++)
	{
		const char *plugin_name = service->loaded_plugins[i];

		lua_getglobal(L, plugin_name);
		if (lua_type(L, -1) == LUA_TTABLE)
		{
			lua_getfield(L, -1, "render");
			if (lua_type(L, -1) == LUA_TFUNCTION)
			{
				if (lua_pcall(L, 0, 0, 0) != LUA_OK)
				{
					fprintf(stderr, "Lua render error in %s: %s\n", plugin_name,
							lua_tostring(L, -1));
					lua_pop(L, 1);
				}
			}
			else
			{
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);
	}
}

void lua_service_apply_config_to_state(struct lua_service *service, struct app_state *state)
{
	if (service == NULL || !service->initialized || state == NULL)
	{
		return;
	}

	int width = lua_runtime_get_config_int(&service->runtime, "window.width", 800);
	int height = lua_runtime_get_config_int(&service->runtime, "window.height", 600);
	app_state_update_dimensions(state, width, height);

	uint32_t bpm =
			(uint32_t)lua_runtime_get_config_int(&service->runtime, "playback.default_bpm", 120);
	app_state_set_bpm(state, bpm);

	state->snap_enabled = lua_runtime_get_config_bool(&service->runtime, "grid.snap_enabled", true);

	printf("Applied config to state: %dx%d, BPM=%d\n", width, height, bpm);
}

const char *lua_service_get_command_for_event(
		struct lua_service *service, struct input_event *event
)
{
	if (service == NULL || !service->initialized || event == NULL)
	{
		return NULL;
	}

	if (event->type != INPUT_EVENT_KEY_DOWN)
	{
		return NULL;
	}

	return lua_command_registry_get_command_for_key(
			&service->command_registry, &event->data.key_down
	);
}

bool lua_service_execute_lua_command(struct lua_service *service, const char *command_name)
{
	if (service == NULL || !service->initialized || command_name == NULL)
	{
		return false;
	}

	return lua_command_registry_execute_command(&service->command_registry, command_name);
}

void lua_service_set_app_controller(
		struct lua_service *service, struct app_controller *controller
)
{
	if (service == NULL || !service->initialized)
	{
		return;
	}

	service->api_context.app_controller = controller;
	lua_api_set_context(&service->api_context);
}
