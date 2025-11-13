#ifndef BOOSTIO_PLATFORM_H
#define BOOSTIO_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>

struct platform_paths
{
	char *binary_dir;
	char *config_dir;
	char *data_dir;
};

void platform_paths_init(struct platform_paths *paths);

void platform_paths_free(struct platform_paths *paths);

bool platform_ensure_directory(const char *path);

#endif
