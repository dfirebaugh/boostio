#include "path_utils.h"

#include <stdio.h>
#include <string.h>

void path_remove_extension(const char *path, char *output, size_t output_size)
{
	if (!path || !output || output_size == 0) {
		if (output && output_size > 0)
			output[0] = '\0';
		return;
	}

	const char *last_dot = strrchr(path, '.');
	const char *last_slash = strrchr(path, '/');

#ifdef _WIN32
	const char *last_backslash = strrchr(path, '\\');
	if (last_backslash && (!last_slash || last_backslash > last_slash))
		last_slash = last_backslash;
#endif

	if (last_dot && (!last_slash || last_dot > last_slash)) {
		size_t len = last_dot - path;
		if (len >= output_size)
			len = output_size - 1;
		strncpy(output, path, len);
		output[len] = '\0';
	} else {
		strncpy(output, path, output_size - 1);
		output[output_size - 1] = '\0';
	}
}

void path_get_directory(const char *path, char *output, size_t output_size)
{
	if (!path || !output || output_size == 0) {
		if (output && output_size > 0)
			output[0] = '\0';
		return;
	}

	const char *last_slash = strrchr(path, '/');

#ifdef _WIN32
	const char *last_backslash = strrchr(path, '\\');
	if (last_backslash && (!last_slash || last_backslash > last_slash))
		last_slash = last_backslash;
#endif

	if (last_slash) {
		size_t len = last_slash - path;
		if (len >= output_size)
			len = output_size - 1;
		strncpy(output, path, len);
		output[len] = '\0';
	} else {
		output[0] = '.';
		output[1] = '\0';
	}
}

void path_get_basename(const char *path, char *output, size_t output_size)
{
	if (!path || !output || output_size == 0) {
		if (output && output_size > 0)
			output[0] = '\0';
		return;
	}

	const char *last_slash = strrchr(path, '/');

#ifdef _WIN32
	const char *last_backslash = strrchr(path, '\\');
	if (last_backslash && (!last_slash || last_backslash > last_slash))
		last_slash = last_backslash;
#endif

	const char *basename = last_slash ? last_slash + 1 : path;

	strncpy(output, basename, output_size - 1);
	output[output_size - 1] = '\0';
}

void path_build_with_extension(
	const char *base_path, const char *extension, char *output, size_t output_size
)
{
	if (!base_path || !extension || !output || output_size == 0) {
		if (output && output_size > 0)
			output[0] = '\0';
		return;
	}

	char path_without_ext[512];
	path_remove_extension(base_path, path_without_ext, sizeof(path_without_ext));

	snprintf(output, output_size, "%s%s", path_without_ext, extension);
}
