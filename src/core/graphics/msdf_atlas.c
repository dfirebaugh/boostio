#include "../../msdf-atlas-gen/atlas_wrapper.h"
#include "msdf_atlas.h"
#include <GL/gl.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct KerningPair
{
	uint32_t left;
	uint32_t right;
	float advance;
};

struct MSDFAtlas
{
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
	if (!fp)
	{
		fprintf(stderr, "Failed to open PNG file: %s\n", filename);
		return NULL;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		fclose(fp);
		return NULL;
	}

	png_infop info = png_create_info_struct(png);
	if (!info)
	{
		png_destroy_read_struct(&png, NULL, NULL);
		fclose(fp);
		return NULL;
	}

	if (setjmp(png_jmpbuf(png)))
	{
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
	if (!image_data)
	{
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	}

	png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * png_height);
	if (!row_pointers)
	{
		free(image_data);
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return NULL;
	}

	for (int y = 0; y < png_height; y++)
	{
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
	if (!f)
	{
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsize < 0)
	{
		fclose(f);
		return NULL;
	}

	char *string = (char *)malloc(fsize + 1);
	if (!string)
	{
		fclose(f);
		return NULL;
	}

	size_t read = fread(string, 1, fsize, f);
	fclose(f);

	if (read != (size_t)fsize)
	{
		free(string);
		return NULL;
	}

	string[fsize] = 0;
	if (size)
		*size = fsize;
	return string;
}

static const char *skip_whitespace(const char *json)
{
	while (*json && (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r'))
	{
		json++;
	}
	return json;
}

static const char *parse_number(const char *json, double *out)
{
	char *end;
	*out = strtod(json, &end);
	return end;
}

static const char *parse_string(const char *json, char *out, size_t max_len)
{
	if (*json != '"')
		return NULL;
	json++;

	size_t i = 0;
	while (*json && *json != '"' && i < max_len - 1)
	{
		if (*json == '\\')
		{
			json++;
			if (!*json)
				return NULL;
		}
		out[i++] = *json++;
	}
	out[i] = 0;

	if (*json != '"')
		return NULL;
	return json + 1;
}

static const char *find_key(const char *json, const char *key)
{
	char key_buf[256];
	const char *p = json;

	while (*p)
	{
		p = skip_whitespace(p);
		if (*p != '"')
		{
			p++;
			continue;
		}

		const char *after_key = parse_string(p, key_buf, sizeof(key_buf));
		if (!after_key)
		{
			p++;
			continue;
		}

		if (strcmp(key_buf, key) == 0)
		{
			p = skip_whitespace(after_key);
			if (*p == ':')
			{
				return skip_whitespace(p + 1);
			}
		}
		p = after_key;
	}

	return NULL;
}

static const char *skip_value(const char *json)
{
	json = skip_whitespace(json);

	if (*json == '"')
	{
		json++;
		while (*json && *json != '"')
		{
			if (*json == '\\')
				json++;
			json++;
		}
		if (*json)
			json++;
	}
	else if (*json == '{' || *json == '[')
	{
		char open = *json;
		char close = (open == '{') ? '}' : ']';
		int depth = 1;
		json++;

		while (*json && depth > 0)
		{
			if (*json == open)
				depth++;
			else if (*json == close)
				depth--;
			else if (*json == '"')
			{
				json++;
				while (*json && *json != '"')
				{
					if (*json == '\\')
						json++;
					json++;
				}
			}
			json++;
		}
	}
	else
	{
		while (*json && *json != ',' && *json != '}' && *json != ']')
		{
			json++;
		}
	}

	return json;
}

struct MSDFAtlas *msdf_atlas_load(const char *json_path, const char *png_path)
{
	size_t json_size;
	char *json_content = read_file(json_path, &json_size);
	if (!json_content)
	{
		fprintf(stderr, "Failed to read JSON file: %s\n", json_path);
		return NULL;
	}

	struct MSDFAtlas *atlas = (struct MSDFAtlas *)malloc(sizeof(struct MSDFAtlas));
	if (!atlas)
	{
		free(json_content);
		return NULL;
	}

	memset(atlas, 0, sizeof(struct MSDFAtlas));
	atlas->glyph_capacity = 256;
	atlas->glyphs = (struct MSDFGlyph *)malloc(sizeof(struct MSDFGlyph) * atlas->glyph_capacity);
	atlas->kerning_capacity = 256;
	atlas->kerning =
			(struct KerningPair *)malloc(sizeof(struct KerningPair) * atlas->kerning_capacity);

	const char *atlas_section = find_key(json_content, "atlas");
	if (atlas_section)
	{
		const char *width_val = find_key(atlas_section, "width");
		const char *height_val = find_key(atlas_section, "height");
		const char *range_val = find_key(atlas_section, "distanceRange");
		const char *size_val = find_key(atlas_section, "size");

		double width, height, range, size;
		if (width_val)
			parse_number(width_val, &width);
		if (height_val)
			parse_number(height_val, &height);
		if (range_val)
			parse_number(range_val, &range);
		if (size_val)
			parse_number(size_val, &size);

		atlas->width = (int)width;
		atlas->height = (int)height;
		atlas->pixel_range = (float)range;
		atlas->font_size = (float)size;
	}

	const char *glyphs_section = find_key(json_content, "glyphs");
	if (glyphs_section && *glyphs_section == '[')
	{
		const char *p = glyphs_section + 1;

		while (*p)
		{
			p = skip_whitespace(p);
			if (*p == ']')
				break;
			if (*p != '{')
			{
				p++;
				continue;
			}

			const char *unicode_val = find_key(p, "unicode");
			if (!unicode_val)
			{
				p = skip_value(p);
				continue;
			}

			double unicode_d;
			parse_number(unicode_val, &unicode_d);
			uint32_t unicode = (uint32_t)unicode_d;

			if (atlas->glyph_count >= atlas->glyph_capacity)
			{
				atlas->glyph_capacity *= 2;
				atlas->glyphs = (struct MSDFGlyph *)realloc(
						atlas->glyphs, sizeof(struct MSDFGlyph) * atlas->glyph_capacity
				);
			}

			struct MSDFGlyph *glyph = &atlas->glyphs[atlas->glyph_count];
			memset(glyph, 0, sizeof(struct MSDFGlyph));
			glyph->codepoint = unicode;

			const char *advance_val = find_key(p, "advance");
			if (advance_val)
			{
				double advance;
				parse_number(advance_val, &advance);
				glyph->advance = (float)advance;
			}

			const char *atlas_bounds = find_key(p, "atlasBounds");
			if (atlas_bounds)
			{
				const char *left_val = find_key(atlas_bounds, "left");
				const char *right_val = find_key(atlas_bounds, "right");
				const char *bottom_val = find_key(atlas_bounds, "bottom");
				const char *top_val = find_key(atlas_bounds, "top");

				double left, right, bottom, top;
				if (left_val)
					parse_number(left_val, &left);
				if (right_val)
					parse_number(right_val, &right);
				if (bottom_val)
					parse_number(bottom_val, &bottom);
				if (top_val)
					parse_number(top_val, &top);

				glyph->x = (float)left / atlas->width;
				glyph->y = (atlas->height - (float)top) / atlas->height;
				glyph->width = (float)(right - left) / atlas->width;
				glyph->height = (float)(top - bottom) / atlas->height;
			}

			const char *plane_bounds = find_key(p, "planeBounds");
			if (plane_bounds)
			{
				const char *left_val = find_key(plane_bounds, "left");
				const char *top_val = find_key(plane_bounds, "top");
				const char *bottom_val = find_key(plane_bounds, "bottom");

				double left, top, bottom;
				if (left_val)
					parse_number(left_val, &left);
				if (top_val)
					parse_number(top_val, &top);
				if (bottom_val)
					parse_number(bottom_val, &bottom);

				glyph->bearing_x = (float)left;
				glyph->bearing_y = (float)top;
				glyph->plane_bottom = (float)bottom;
			}

			atlas->glyph_count++;

			p = skip_value(p);
			p = skip_whitespace(p);
			if (*p == ',')
				p++;
		}
	}

	const char *kerning_section = find_key(json_content, "kerning");
	if (kerning_section && *kerning_section == '[')
	{
		const char *p = kerning_section + 1;

		while (*p)
		{
			p = skip_whitespace(p);
			if (*p == ']')
				break;
			if (*p != '{')
			{
				p++;
				continue;
			}

			const char *u1_val = find_key(p, "unicode1");
			const char *u2_val = find_key(p, "unicode2");
			const char *adv_val = find_key(p, "advance");

			if (u1_val && u2_val && adv_val)
			{
				if (atlas->kerning_count >= atlas->kerning_capacity)
				{
					atlas->kerning_capacity *= 2;
					atlas->kerning = (struct KerningPair *)realloc(
							atlas->kerning, sizeof(struct KerningPair) * atlas->kerning_capacity
					);
				}

				double u1, u2, adv;
				parse_number(u1_val, &u1);
				parse_number(u2_val, &u2);
				parse_number(adv_val, &adv);

				atlas->kerning[atlas->kerning_count].left = (uint32_t)u1;
				atlas->kerning[atlas->kerning_count].right = (uint32_t)u2;
				atlas->kerning[atlas->kerning_count].advance = (float)adv;
				atlas->kerning_count++;
			}

			p = skip_value(p);
			p = skip_whitespace(p);
			if (*p == ',')
				p++;
		}
	}

	free(json_content);

	int png_width, png_height;
	unsigned char *image_data = load_png(png_path, &png_width, &png_height);
	if (!image_data)
	{
		fprintf(stderr, "Failed to load PNG: %s\n", png_path);
		free(atlas->glyphs);
		free(atlas->kerning);
		free(atlas);
		return NULL;
	}

	glGenTextures(1, &atlas->texture_id);
	glBindTexture(GL_TEXTURE_2D, atlas->texture_id);
	glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA, png_width, png_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
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
	if (!atlas)
	{
		return;
	}

	if (atlas->texture_id)
	{
		glDeleteTextures(1, &atlas->texture_id);
	}

	if (atlas->glyphs)
	{
		free(atlas->glyphs);
	}

	if (atlas->kerning)
	{
		free(atlas->kerning);
	}

	free(atlas);
}

const struct MSDFGlyph *msdf_atlas_get_glyph(const struct MSDFAtlas *atlas, uint32_t codepoint)
{
	if (!atlas || !atlas->glyphs)
	{
		return NULL;
	}

	for (int i = 0; i < atlas->glyph_count; i++)
	{
		if (atlas->glyphs[i].codepoint == codepoint)
		{
			return &atlas->glyphs[i];
		}
	}

	return NULL;
}

float msdf_atlas_get_kerning(const struct MSDFAtlas *atlas, uint32_t left, uint32_t right)
{
	if (!atlas || !atlas->kerning)
	{
		return 0.0f;
	}

	for (int i = 0; i < atlas->kerning_count; i++)
	{
		if (atlas->kerning[i].left == left && atlas->kerning[i].right == right)
		{
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
	if (!atlas || !text)
	{
		return 0.0f;
	}

	float width = 0.0f;
	uint32_t prev_char = 0;

	for (const char *p = text; *p; p++)
	{
		uint32_t char_code = (uint32_t)(unsigned char)*p;

		if (prev_char)
		{
			width += msdf_atlas_get_kerning(atlas, prev_char, char_code) * size;
		}

		const struct MSDFGlyph *glyph = msdf_atlas_get_glyph(atlas, char_code);
		if (glyph)
		{
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
	if (!font_path || !png_path || !json_path)
	{
		return false;
	}

	if (!file_exists(font_path))
	{
		fprintf(stderr, "Font file not found: %s\n", font_path);
		return false;
	}

	if (file_exists(json_path) && file_exists(png_path))
	{
		return true;
	}

	MSDFAtlasGen *gen = atlas_generator_create();
	if (!gen)
	{
		fprintf(stderr, "Failed to create atlas generator\n");
		return false;
	}

	int result = atlas_generator_generate_mtsdf(gen, font_path, png_path, json_path, 32.0, 4.0);

	if (result != 0)
	{
		const char *error = atlas_generator_get_error(gen);
		fprintf(stderr, "Atlas generation failed: %s\n", error ? error : "unknown error");
		atlas_generator_destroy(gen);
		return false;
	}

	atlas_generator_destroy(gen);
	return true;
}
