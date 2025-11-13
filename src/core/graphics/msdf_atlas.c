#include "msdf_atlas.h"
#include "../../msdf-atlas-gen/atlas_wrapper.h"
#include <cJSON.h>
#include <GL/gl.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct KerningPair {
	uint32_t left;
	uint32_t right;
	float advance;
};

struct MSDFAtlas {
	unsigned int texture_id;
	int width;
	int height;
	float pixel_range;
	float font_size;

	struct MSDFGlyph *glyphs;
	int glyph_count;
	int glyph_capacity;

	struct KerningPair *kerning;
	int kerning_count;
	int kerning_capacity;
};

static unsigned char *load_png(const char *filename, int *width, int *height)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		fprintf(stderr, "Failed to open PNG file: %s\n", filename);
		return NULL;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fclose(fp);
		return NULL;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		png_destroy_read_struct(&png, NULL, NULL);
		fclose(fp);
		return NULL;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	}

	png_init_io(png, fp);
	png_read_info(png, info);

	int png_width = png_get_image_width(png, info);
	int png_height = png_get_image_height(png, info);
	png_byte color_type = png_get_color_type(png, info);
	png_byte bit_depth = png_get_bit_depth(png, info);

	if (bit_depth == 16)
		png_set_strip_16(png);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
	    color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	size_t row_bytes = png_get_rowbytes(png, info);
	unsigned char *image_data = (unsigned char *)malloc(row_bytes * png_height);
	if (!image_data) {
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	}

	png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * png_height);
	if (!row_pointers) {
		free(image_data);
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	}

	for (int y = 0; y < png_height; y++) {
		row_pointers[y] = image_data + y * row_bytes;
	}

	png_read_image(png, row_pointers);

	free(row_pointers);
	png_destroy_read_struct(&png, &info, NULL);
	fclose(fp);

	*width = png_width;
	*height = png_height;

	return image_data;
}

static char *read_file(const char *path, size_t *size)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsize < 0) {
		fclose(f);
		return NULL;
	}

	char *string = (char *)malloc(fsize + 1);
	if (!string) {
		fclose(f);
		return NULL;
	}

	size_t read = fread(string, 1, fsize, f);
	fclose(f);

	if (read != (size_t)fsize) {
		free(string);
		return NULL;
	}

	string[fsize] = 0;
	if (size)
		*size = fsize;
	return string;
}

struct MSDFAtlas *msdf_atlas_load(const char *json_path, const char *png_path)
{
	size_t json_size;
	char *json_content = read_file(json_path, &json_size);
	if (!json_content) {
		fprintf(stderr, "Failed to read JSON file: %s\n", json_path);
		return NULL;
	}

	cJSON *root = cJSON_Parse(json_content);
	free(json_content);

	if (!root) {
		fprintf(stderr, "Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
		return NULL;
	}

	struct MSDFAtlas *atlas = (struct MSDFAtlas *)malloc(sizeof(struct MSDFAtlas));
	if (!atlas) {
		cJSON_Delete(root);
		return NULL;
	}

	memset(atlas, 0, sizeof(struct MSDFAtlas));
	atlas->glyph_capacity = 256;
	atlas->glyphs =
		(struct MSDFGlyph *)malloc(sizeof(struct MSDFGlyph) * atlas->glyph_capacity);
	atlas->kerning_capacity = 256;
	atlas->kerning =
		(struct KerningPair *)malloc(sizeof(struct KerningPair) * atlas->kerning_capacity);

	cJSON *atlas_obj = cJSON_GetObjectItem(root, "atlas");
	if (atlas_obj) {
		cJSON *width = cJSON_GetObjectItem(atlas_obj, "width");
		cJSON *height = cJSON_GetObjectItem(atlas_obj, "height");
		cJSON *distance_range = cJSON_GetObjectItem(atlas_obj, "distanceRange");
		cJSON *size = cJSON_GetObjectItem(atlas_obj, "size");

		if (width)
			atlas->width = width->valueint;
		if (height)
			atlas->height = height->valueint;
		if (distance_range)
			atlas->pixel_range = (float)distance_range->valuedouble;
		if (size)
			atlas->font_size = (float)size->valuedouble;
	}

	cJSON *glyphs = cJSON_GetObjectItem(root, "glyphs");
	if (cJSON_IsArray(glyphs)) {
		cJSON *glyph_obj;
		cJSON_ArrayForEach(glyph_obj, glyphs)
		{
			cJSON *unicode = cJSON_GetObjectItem(glyph_obj, "unicode");
			if (!unicode)
				continue;

			if (atlas->glyph_count >= atlas->glyph_capacity) {
				atlas->glyph_capacity *= 2;
				atlas->glyphs = (struct MSDFGlyph *)realloc(
					atlas->glyphs,
					sizeof(struct MSDFGlyph) * atlas->glyph_capacity
				);
			}

			struct MSDFGlyph *glyph = &atlas->glyphs[atlas->glyph_count];
			memset(glyph, 0, sizeof(struct MSDFGlyph));
			glyph->codepoint = (uint32_t)unicode->valueint;

			cJSON *advance = cJSON_GetObjectItem(glyph_obj, "advance");
			if (advance)
				glyph->advance = (float)advance->valuedouble;

			cJSON *atlas_bounds = cJSON_GetObjectItem(glyph_obj, "atlasBounds");
			if (atlas_bounds) {
				cJSON *left = cJSON_GetObjectItem(atlas_bounds, "left");
				cJSON *right = cJSON_GetObjectItem(atlas_bounds, "right");
				cJSON *bottom = cJSON_GetObjectItem(atlas_bounds, "bottom");
				cJSON *top = cJSON_GetObjectItem(atlas_bounds, "top");

				if (left && right && bottom && top) {
					double left_val = left->valuedouble;
					double right_val = right->valuedouble;
					double top_val = top->valuedouble;
					double bottom_val = bottom->valuedouble;

					glyph->x = (float)left_val / atlas->width;
					glyph->y = (atlas->height - (float)top_val) / atlas->height;
					glyph->width = (float)(right_val - left_val) / atlas->width;
					glyph->height =
						(float)(top_val - bottom_val) / atlas->height;
				}
			}

			cJSON *plane_bounds = cJSON_GetObjectItem(glyph_obj, "planeBounds");
			if (plane_bounds) {
				cJSON *left = cJSON_GetObjectItem(plane_bounds, "left");
				cJSON *top = cJSON_GetObjectItem(plane_bounds, "top");
				cJSON *bottom = cJSON_GetObjectItem(plane_bounds, "bottom");

				if (left)
					glyph->bearing_x = (float)left->valuedouble;
				if (top)
					glyph->bearing_y = (float)top->valuedouble;
				if (bottom)
					glyph->plane_bottom = (float)bottom->valuedouble;
			}

			atlas->glyph_count++;
		}
	}

	cJSON *kerning = cJSON_GetObjectItem(root, "kerning");
	if (cJSON_IsArray(kerning)) {
		cJSON *kerning_obj;
		cJSON_ArrayForEach(kerning_obj, kerning)
		{
			cJSON *u1 = cJSON_GetObjectItem(kerning_obj, "unicode1");
			cJSON *u2 = cJSON_GetObjectItem(kerning_obj, "unicode2");
			cJSON *adv = cJSON_GetObjectItem(kerning_obj, "advance");

			if (u1 && u2 && adv) {
				if (atlas->kerning_count >= atlas->kerning_capacity) {
					atlas->kerning_capacity *= 2;
					atlas->kerning = (struct KerningPair *)realloc(
						atlas->kerning,
						sizeof(struct KerningPair) * atlas->kerning_capacity
					);
				}

				atlas->kerning[atlas->kerning_count].left = (uint32_t)u1->valueint;
				atlas->kerning[atlas->kerning_count].right = (uint32_t)u2->valueint;
				atlas->kerning[atlas->kerning_count].advance =
					(float)adv->valuedouble;
				atlas->kerning_count++;
			}
		}
	}

	cJSON_Delete(root);

	int png_width, png_height;
	unsigned char *image_data = load_png(png_path, &png_width, &png_height);
	if (!image_data) {
		fprintf(stderr, "Failed to load PNG: %s\n", png_path);
		free(atlas->glyphs);
		free(atlas->kerning);
		free(atlas);
		return NULL;
	}

	glGenTextures(1, &atlas->texture_id);
	glBindTexture(GL_TEXTURE_2D, atlas->texture_id);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		png_width,
		png_height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	free(image_data);

	return atlas;
}

void msdf_atlas_destroy(struct MSDFAtlas *atlas)
{
	if (!atlas) {
		return;
	}

	if (atlas->texture_id) {
		glDeleteTextures(1, &atlas->texture_id);
	}

	if (atlas->glyphs) {
		free(atlas->glyphs);
	}

	if (atlas->kerning) {
		free(atlas->kerning);
	}

	free(atlas);
}

const struct MSDFGlyph *msdf_atlas_get_glyph(const struct MSDFAtlas *atlas, uint32_t codepoint)
{
	if (!atlas || !atlas->glyphs) {
		return NULL;
	}

	for (int i = 0; i < atlas->glyph_count; i++) {
		if (atlas->glyphs[i].codepoint == codepoint) {
			return &atlas->glyphs[i];
		}
	}

	return NULL;
}

float msdf_atlas_get_kerning(const struct MSDFAtlas *atlas, uint32_t left, uint32_t right)
{
	if (!atlas || !atlas->kerning) {
		return 0.0f;
	}

	for (int i = 0; i < atlas->kerning_count; i++) {
		if (atlas->kerning[i].left == left && atlas->kerning[i].right == right) {
			return atlas->kerning[i].advance;
		}
	}

	return 0.0f;
}

unsigned int msdf_atlas_get_texture_id(const struct MSDFAtlas *atlas)
{
	return atlas ? atlas->texture_id : 0;
}

int msdf_atlas_get_width(const struct MSDFAtlas *atlas)
{
	return atlas ? atlas->width : 0;
}

int msdf_atlas_get_height(const struct MSDFAtlas *atlas)
{
	return atlas ? atlas->height : 0;
}

float msdf_atlas_get_pixel_range(const struct MSDFAtlas *atlas)
{
	return atlas ? atlas->pixel_range : 0.0f;
}

float msdf_atlas_get_font_size(const struct MSDFAtlas *atlas)
{
	return atlas ? atlas->font_size : 0.0f;
}

float msdf_atlas_measure_text(const struct MSDFAtlas *atlas, const char *text, float size)
{
	if (!atlas || !text) {
		return 0.0f;
	}

	float width = 0.0f;
	uint32_t prev_char = 0;

	for (const char *p = text; *p; p++) {
		uint32_t char_code = (uint32_t)(unsigned char)*p;

		if (prev_char) {
			width += msdf_atlas_get_kerning(atlas, prev_char, char_code) * size;
		}

		const struct MSDFGlyph *glyph = msdf_atlas_get_glyph(atlas, char_code);
		if (glyph) {
			width += glyph->advance * size;
		}

		prev_char = char_code;
	}

	return width;
}

static bool file_exists(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0;
}

bool msdf_atlas_generate(const char *font_path, const char *png_path, const char *json_path)
{
	if (!font_path || !png_path || !json_path) {
		return false;
	}

	if (!file_exists(font_path)) {
		fprintf(stderr, "Font file not found: %s\n", font_path);
		return false;
	}

	if (file_exists(json_path) && file_exists(png_path)) {
		return true;
	}

	MSDFAtlasGen *gen = atlas_generator_create();
	if (!gen) {
		fprintf(stderr, "Failed to create atlas generator\n");
		return false;
	}

	int result = atlas_generator_generate_mtsdf(gen, font_path, png_path, json_path, 32.0, 4.0);

	if (result != 0) {
		const char *error = atlas_generator_get_error(gen);
		fprintf(stderr, "Atlas generation failed: %s\n", error ? error : "unknown error");
		atlas_generator_destroy(gen);
		return false;
	}

	atlas_generator_destroy(gen);
	return true;
}
