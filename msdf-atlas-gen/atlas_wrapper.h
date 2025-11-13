#ifndef ATLAS_WRAPPER_H
#define ATLAS_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MSDFAtlasGen MSDFAtlasGen;

MSDFAtlasGen* atlas_generator_create(void);
void atlas_generator_destroy(MSDFAtlasGen* gen);

int atlas_generator_generate_mtsdf(
		MSDFAtlasGen* gen,
		const char* font_path,
		const char* png_path,
		const char* json_path,
		double font_size,
		double pixel_range
	);

const char* atlas_generator_get_error(MSDFAtlasGen* gen);

#ifdef __cplusplus
}
#endif

#endif

