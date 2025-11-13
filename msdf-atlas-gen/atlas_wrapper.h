#ifndef ATLAS_WRAPPER_H
#define ATLAS_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

struct msdf_atlas_gen;

struct msdf_atlas_gen* atlas_generator_create(void);
void atlas_generator_destroy(struct msdf_atlas_gen* gen);

int atlas_generator_generate_mtsdf(
		struct msdf_atlas_gen* gen,
		const char* font_path,
		const char* png_path,
		const char* json_path,
		double font_size,
		double pixel_range
	);

const char* atlas_generator_get_error(struct msdf_atlas_gen* gen);

#ifdef __cplusplus
}
#endif

#endif

