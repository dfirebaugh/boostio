#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <stdbool.h>
#include <stddef.h>

void path_remove_extension(const char *path, char *output, size_t output_size);

void path_get_directory(const char *path, char *output, size_t output_size);

void path_get_basename(const char *path, char *output, size_t output_size);

void path_build_with_extension(
		const char *base_path,
		const char *extension,
		char *output,
		size_t output_size
);

#endif
