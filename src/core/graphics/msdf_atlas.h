#ifndef BOOSTIO_MSDF_ATLAS_H
#define BOOSTIO_MSDF_ATLAS_H

#include <stdbool.h>
#include <stdint.h>

struct msdf_atlas;
struct msdf_glyph;

struct msdf_atlas *msdf_atlas_load(const char *json_path, const char *png_path);
void msdf_atlas_destroy(struct msdf_atlas *atlas);

float msdf_atlas_measure_text(const struct msdf_atlas *atlas, const char *text, float size);
const struct msdf_glyph *msdf_atlas_get_glyph(const struct msdf_atlas *atlas, uint32_t codepoint);
float msdf_atlas_get_kerning(const struct msdf_atlas *atlas, uint32_t left, uint32_t right);

unsigned int msdf_atlas_get_texture_id(const struct msdf_atlas *atlas);
int msdf_atlas_get_width(const struct msdf_atlas *atlas);
int msdf_atlas_get_height(const struct msdf_atlas *atlas);
float msdf_atlas_get_pixel_range(const struct msdf_atlas *atlas);
float msdf_atlas_get_font_size(const struct msdf_atlas *atlas);

bool msdf_atlas_generate(const char *font_path, const char *png_path, const char *json_path);

struct msdf_glyph {
	uint32_t codepoint;
	float x, y;
	float width, height;
	float advance;
	float bearing_x, bearing_y;
	float plane_bottom;
};

#endif
