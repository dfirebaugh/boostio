#ifndef BOOSTIO_MSDF_ATLAS_H
#define BOOSTIO_MSDF_ATLAS_H

#include <stdbool.h>
#include <stdint.h>

struct MSDFAtlas;
struct MSDFGlyph;

struct MSDFAtlas *msdf_atlas_load(const char *json_path, const char *png_path);
void msdf_atlas_destroy(struct MSDFAtlas *atlas);

float msdf_atlas_measure_text(const struct MSDFAtlas *atlas, const char *text, float size);
const struct MSDFGlyph *msdf_atlas_get_glyph(const struct MSDFAtlas *atlas, uint32_t codepoint);
float msdf_atlas_get_kerning(const struct MSDFAtlas *atlas, uint32_t left, uint32_t right);

unsigned int msdf_atlas_get_texture_id(const struct MSDFAtlas *atlas);
int msdf_atlas_get_width(const struct MSDFAtlas *atlas);
int msdf_atlas_get_height(const struct MSDFAtlas *atlas);
float msdf_atlas_get_pixel_range(const struct MSDFAtlas *atlas);
float msdf_atlas_get_font_size(const struct MSDFAtlas *atlas);

bool msdf_atlas_generate(const char *font_path, const char *png_path, const char *json_path);

struct MSDFGlyph
{
	uint32_t codepoint;
	float x, y;
	float width, height;
	float advance;
	float bearing_x, bearing_y;
	float plane_bottom;
};

#endif
