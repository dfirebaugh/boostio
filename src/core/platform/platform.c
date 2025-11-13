#include "core/platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#include <windows.h>
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#elif defined(__linux__)
#define PLATFORM_LINUX
#endif

static char *get_env_or_default(const char *env_name, const char *default_value)
{
	const char *env_value = getenv(env_name);
	if (env_value != NULL)
	{
		return strdup(env_value);
	}
	return strdup(default_value);
}

static char *expand_path(const char *path)
{
	if (path == NULL)
	{
		return NULL;
	}

#if defined(PLATFORM_WINDOWS)
	char expanded[1024];
	DWORD result = ExpandEnvironmentStringsA(path, expanded, sizeof(expanded));
	if (result > 0 && result < sizeof(expanded))
	{
		return strdup(expanded);
	}
	return strdup(path);
#else
	if (path[0] == '~')
	{
		const char *home = getenv("HOME");
		if (home != NULL)
		{
			size_t home_len = strlen(home);
			size_t path_len = strlen(path + 1);
			char *expanded = malloc(home_len + path_len + 1);
			if (expanded != NULL)
			{
				memcpy(expanded, home, home_len);
				memcpy(expanded + home_len, path + 1, path_len + 1);
				return expanded;
			}
		}
	}
	return strdup(path);
#endif
}

static char *get_binary_dir(void)
{
	const char *override = getenv("BOOSTIO_BINARY_DIR");
	if (override != NULL)
	{
		return expand_path(override);
	}

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
	return expand_path("~/.local/bin");
#elif defined(PLATFORM_WINDOWS)
	return expand_path("%LOCALAPPDATA%\\Programs\\Boostio");
#else
	return strdup(".");
#endif
}

static char *get_config_dir(void)
{
	const char *override = getenv("BOOSTIO_CONFIG_DIR");
	if (override != NULL)
	{
		return expand_path(override);
	}

#if defined(PLATFORM_LINUX)
	return expand_path("~/.config/boostio");
#elif defined(PLATFORM_MACOS)
	return expand_path("~/Library/Application Support/Boostio");
#elif defined(PLATFORM_WINDOWS)
	return expand_path("%APPDATA%\\Boostio");
#else
	return strdup(".");
#endif
}

static char *get_data_dir(void)
{
	const char *override = getenv("BOOSTIO_DATA_DIR");
	if (override != NULL)
	{
		return expand_path(override);
	}

#if defined(PLATFORM_LINUX)
	return expand_path("~/.local/share/boostio");
#elif defined(PLATFORM_MACOS)
	return expand_path("~/Library/Application Support/Boostio");
#elif defined(PLATFORM_WINDOWS)
	return expand_path("%APPDATA%\\Boostio");
#else
	return strdup(".");
#endif
}

void platform_paths_init(struct platform_paths *paths)
{
	if (paths == NULL)
	{
		return;
	}

	paths->binary_dir = get_binary_dir();
	paths->config_dir = get_config_dir();
	paths->data_dir = get_data_dir();
}

void platform_paths_free(struct platform_paths *paths)
{
	if (paths == NULL)
	{
		return;
	}

	free(paths->binary_dir);
	free(paths->config_dir);
	free(paths->data_dir);
	paths->binary_dir = NULL;
	paths->config_dir = NULL;
	paths->data_dir = NULL;
}
