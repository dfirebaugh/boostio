#include "graphics.h"
#include "color.h"
#include "msdf_atlas.h"
#include "primitive_buffer.h"
#include "shader.h"
#include "window.h"

#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *msdf_vertex_shader = "#version 330 core\n"
					"layout (location = 0) in vec2 aPos;\n"
					"layout (location = 1) in vec2 aTexCoord;\n"
					"layout (location = 2) in vec4 aColor;\n"
					"\n"
					"out vec2 TexCoord;\n"
					"out vec4 Color;\n"
					"\n"
					"uniform mat4 projection;\n"
					"\n"
					"void main() {\n"
					"    gl_Position = projection * vec4(aPos, 0.0, 1.0);\n"
					"    TexCoord = aTexCoord;\n"
					"    Color = aColor;\n"
					"}\n";

static const char *msdf_fragment_shader =
	"#version 330 core\n"
	"in vec2 TexCoord;\n"
	"in vec4 Color;\n"
	"out vec4 FragColor;\n"
	"\n"
	"uniform sampler2D msdfTexture;\n"
	"uniform float pxRange;\n"
	"uniform vec2 texSize;\n"
	"\n"
	"float median(float r, float g, float b) {\n"
	"    return max(min(r, g), min(max(r, g), b));\n"
	"}\n"
	"\n"
	"float screenPxRange() {\n"
	"    vec2 unitRange = vec2(pxRange)/texSize;\n"
	"    vec2 screenTexSize = vec2(1.0)/fwidth(TexCoord);\n"
	"    return max(0.5*dot(unitRange, screenTexSize), 1.0);\n"
	"}\n"
	"\n"
	"void main() {\n"
	"    vec3 msd = texture(msdfTexture, TexCoord).rgb;\n"
	"    float sd = median(msd.r, msd.g, msd.b);\n"
	"    float screenPxDistance = screenPxRange()*(sd - 0.5);\n"
	"    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);\n"
	"    FragColor = vec4(Color.rgb, Color.a * opacity);\n"
	"}\n";

struct graphics {
	struct window *window;
	SDL_GLContext gl_context;
	struct color current_color;
	struct msdf_atlas *font_atlas;

	struct shader *msdf_shader;
	unsigned int text_vao;
	unsigned int text_vbo;
	unsigned int text_ebo;

	struct primitive_buffer *primitive_buffer;

	uint64_t last_fps_update_time;
	uint32_t frame_count;
	float current_fps;
};

struct graphics *graphics_create(struct window *window)
{
	if (!window) {
		fprintf(stderr, "Window is NULL\n");
		return NULL;
	}

	struct graphics *graphics = (struct graphics *)malloc(sizeof(struct graphics));
	if (!graphics) {
		fprintf(stderr, "Failed to allocate graphics context\n");
		return NULL;
	}

	memset(graphics, 0, sizeof(struct graphics));
	graphics->window = window;
	graphics->current_color = COLOR_WHITE;
	graphics->last_fps_update_time = SDL_GetPerformanceCounter();
	graphics->frame_count = 0;
	graphics->current_fps = 0.0f;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	graphics->gl_context = SDL_GL_CreateContext(window->window);
	if (!graphics->gl_context) {
		fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		free(graphics);
		return NULL;
	}

	SDL_GL_MakeCurrent(window->window, graphics->gl_context);
	SDL_GL_SetSwapInterval(1);

	if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
		fprintf(stderr, "Failed to initialize GLAD\n");
		SDL_GL_DestroyContext(graphics->gl_context);
		free(graphics);
		return NULL;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	graphics->msdf_shader = shader_create(msdf_vertex_shader, msdf_fragment_shader);
	if (!graphics->msdf_shader) {
		fprintf(stderr, "Failed to create MSDF shader\n");
	}

	glGenVertexArrays(1, &graphics->text_vao);
	glGenBuffers(1, &graphics->text_vbo);
	glGenBuffers(1, &graphics->text_ebo);

	graphics->primitive_buffer = primitive_buffer_create();
	if (!graphics->primitive_buffer) {
		fprintf(stderr, "Failed to create primitive buffer\n");
	}

	return graphics;
}

void graphics_destroy(struct graphics *graphics)
{
	if (!graphics) {
		return;
	}

	if (graphics->font_atlas) {
		msdf_atlas_destroy(graphics->font_atlas);
	}

	if (graphics->primitive_buffer) {
		primitive_buffer_destroy(graphics->primitive_buffer);
	}

	if (graphics->msdf_shader) {
		shader_destroy(graphics->msdf_shader);
	}

	if (graphics->text_vao) {
		glDeleteVertexArrays(1, &graphics->text_vao);
	}

	if (graphics->text_vbo) {
		glDeleteBuffers(1, &graphics->text_vbo);
	}

	if (graphics->text_ebo) {
		glDeleteBuffers(1, &graphics->text_ebo);
	}

	if (graphics->gl_context) {
		SDL_GL_DestroyContext(graphics->gl_context);
	}

	free(graphics);
}

void graphics_clear(struct graphics *graphics, struct color color)
{
	if (!graphics) {
		return;
	}

	glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (graphics->primitive_buffer) {
		primitive_buffer_begin(graphics->primitive_buffer);
	}
}

void graphics_set_color(struct graphics *graphics, struct color color)
{
	if (!graphics) {
		return;
	}

	graphics->current_color = color;
}

void graphics_fill_rect(struct graphics *graphics, int x, int y, int width, int height)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		struct color no_outline = {0, 0, 0, 0};
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			graphics->current_color,
			0.0f,
			0.0f,
			no_outline
		);
	}
}

void graphics_fill_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	struct color outline_color,
	int outline_width
)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			graphics->current_color,
			0.0f,
			(float)outline_width,
			outline_color
		);
	}
}

void graphics_draw_rect(struct graphics *graphics, int x, int y, int width, int height)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		struct color transparent = {0, 0, 0, 0};
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			transparent,
			0.0f,
			1.0f,
			graphics->current_color
		);
	}
}

void graphics_draw_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	struct color outline_color,
	int outline_width
)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		struct color transparent = {0, 0, 0, 0};
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			transparent,
			0.0f,
			(float)outline_width,
			outline_color
		);
	}
}

void graphics_fill_rounded_rect(
	struct graphics *graphics, int x, int y, int width, int height, int radius
)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		struct color no_outline = {0, 0, 0, 0};
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			graphics->current_color,
			(float)radius,
			0.0f,
			no_outline
		);
	}
}

void graphics_fill_rounded_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	int radius,
	struct color outline_color,
	int outline_width
)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			graphics->current_color,
			(float)radius,
			(float)outline_width,
			outline_color
		);
	}
}

void graphics_draw_rounded_rect(
	struct graphics *graphics, int x, int y, int width, int height, int radius
)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		struct color transparent = {0, 0, 0, 0};
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			transparent,
			(float)radius,
			1.0f,
			graphics->current_color
		);
	}
}

void graphics_draw_rounded_rect_outlined(
	struct graphics *graphics,
	int x,
	int y,
	int width,
	int height,
	int radius,
	struct color outline_color,
	int outline_width
)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		struct color transparent = {0, 0, 0, 0};
		primitive_buffer_add_rect(
			graphics->primitive_buffer,
			(float)x,
			(float)y,
			(float)width,
			(float)height,
			transparent,
			(float)radius,
			(float)outline_width,
			outline_color
		);
	}
}

void graphics_draw_line(struct graphics *graphics, int x1, int y1, int x2, int y2)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		primitive_buffer_add_line(
			graphics->primitive_buffer,
			(float)x1,
			(float)y1,
			(float)x2,
			(float)y2,
			graphics->current_color
		);
	}
}

void graphics_present(struct graphics *graphics)
{
	if (!graphics) {
		return;
	}

	if (graphics->primitive_buffer) {
		int window_width, window_height;
		SDL_GetWindowSize(graphics->window->window, &window_width, &window_height);
		primitive_buffer_render(graphics->primitive_buffer, window_width, window_height);
	}

	SDL_GL_SwapWindow(graphics->window->window);

	graphics->frame_count++;

	uint64_t current_time = SDL_GetPerformanceCounter();
	uint64_t frequency = SDL_GetPerformanceFrequency();
	double elapsed = (double)(current_time - graphics->last_fps_update_time) / frequency;

	if (elapsed >= 0.5) {
		graphics->current_fps = graphics->frame_count / elapsed;
		graphics->frame_count = 0;
		graphics->last_fps_update_time = current_time;
	}
}

bool graphics_should_close(const struct graphics *graphics)
{
	if (!graphics || !graphics->window) {
		return true;
	}

	return graphics->window->should_close;
}

bool graphics_load_font(struct graphics *graphics, const char *json_path, const char *png_path)
{
	if (!graphics) {
		return false;
	}

	if (graphics->font_atlas) {
		msdf_atlas_destroy(graphics->font_atlas);
	}

	graphics->font_atlas = msdf_atlas_load(json_path, png_path);
	return graphics->font_atlas != NULL;
}

void graphics_draw_text(struct graphics *graphics, const char *text, int x, int y, int size)
{
	if (!graphics || !text) {
		return;
	}

	if (!graphics->font_atlas || !graphics->msdf_shader) {
		return;
	}

	int window_width, window_height;
	SDL_GetWindowSize(graphics->window->window, &window_width, &window_height);

	if (graphics->primitive_buffer) {
		primitive_buffer_render(graphics->primitive_buffer, window_width, window_height);
		primitive_buffer_begin(graphics->primitive_buffer);
	}

	glViewport(0, 0, window_width, window_height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float projection[16];
	ortho_projection(0, (float)window_width, (float)window_height, 0, -1.0f, 1.0f, projection);

	shader_use(graphics->msdf_shader);
	shader_set_mat4(graphics->msdf_shader, "projection", projection);
	shader_set_float(
		graphics->msdf_shader, "pxRange", msdf_atlas_get_pixel_range(graphics->font_atlas)
	);
	shader_set_vec2(
		graphics->msdf_shader,
		"texSize",
		(float)msdf_atlas_get_width(graphics->font_atlas),
		(float)msdf_atlas_get_height(graphics->font_atlas)
	);
	shader_set_int(graphics->msdf_shader, "msdfTexture", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, msdf_atlas_get_texture_id(graphics->font_atlas));

	float *vertices = NULL;
	unsigned int *indices = NULL;
	int vertex_count = 0;
	int index_count = 0;
	int vertex_capacity = 256;
	int index_capacity = 384;

	vertices = (float *)malloc(sizeof(float) * vertex_capacity * 8);
	indices = (unsigned int *)malloc(sizeof(unsigned int) * index_capacity);

	float cursor_x = (float)x;
	float baseline_y = (float)y;
	uint32_t prev_char = 0;

	float atlas_font_size = msdf_atlas_get_font_size(graphics->font_atlas);
	int atlas_width = msdf_atlas_get_width(graphics->font_atlas);

	for (const char *p = text; *p; p++) {
		uint32_t char_code = (uint32_t)(unsigned char)*p;

		if (prev_char) {
			float kerning =
				msdf_atlas_get_kerning(graphics->font_atlas, prev_char, char_code);
			cursor_x += kerning * (float)size;
		}

		const struct msdf_glyph *glyph =
			msdf_atlas_get_glyph(graphics->font_atlas, char_code);
		if (!glyph) {
			prev_char = char_code;
			continue;
		}

		if (vertex_count + 4 > vertex_capacity) {
			vertex_capacity *= 2;
			vertices = (float *)realloc(vertices, sizeof(float) * vertex_capacity * 8);
		}

		if (index_count + 6 > index_capacity) {
			index_capacity *= 2;
			indices = (unsigned int *)realloc(
				indices, sizeof(unsigned int) * index_capacity
			);
		}

		float atlas_pixel_width = glyph->width * (float)atlas_width;
		float glyph_height_normalized = glyph->bearing_y - glyph->plane_bottom;
		float glyph_width = atlas_pixel_width * ((float)size / atlas_font_size);
		float glyph_height = glyph_height_normalized * (float)size;

		float glyph_x = cursor_x + (glyph->bearing_x * (float)size);
		float glyph_y = baseline_y - (glyph->bearing_y * (float)size);

		float r = graphics->current_color.r / 255.0f;
		float g = graphics->current_color.g / 255.0f;
		float b = graphics->current_color.b / 255.0f;
		float a = graphics->current_color.a / 255.0f;

		int base_vertex = vertex_count;
		float *v = &vertices[vertex_count * 8];

		v[0] = glyph_x;
		v[1] = glyph_y;
		v[2] = glyph->x;
		v[3] = glyph->y;
		v[4] = r;
		v[5] = g;
		v[6] = b;
		v[7] = a;

		v[8] = glyph_x + glyph_width;
		v[9] = glyph_y;
		v[10] = glyph->x + glyph->width;
		v[11] = glyph->y;
		v[12] = r;
		v[13] = g;
		v[14] = b;
		v[15] = a;

		v[16] = glyph_x + glyph_width;
		v[17] = glyph_y + glyph_height;
		v[18] = glyph->x + glyph->width;
		v[19] = glyph->y + glyph->height;
		v[20] = r;
		v[21] = g;
		v[22] = b;
		v[23] = a;

		v[24] = glyph_x;
		v[25] = glyph_y + glyph_height;
		v[26] = glyph->x;
		v[27] = glyph->y + glyph->height;
		v[28] = r;
		v[29] = g;
		v[30] = b;
		v[31] = a;

		indices[index_count++] = base_vertex + 0;
		indices[index_count++] = base_vertex + 1;
		indices[index_count++] = base_vertex + 2;
		indices[index_count++] = base_vertex + 0;
		indices[index_count++] = base_vertex + 2;
		indices[index_count++] = base_vertex + 3;

		vertex_count += 4;
		cursor_x += glyph->advance * (float)size;
		prev_char = char_code;
	}

	if (vertex_count > 0) {
		glBindVertexArray(graphics->text_vao);

		glBindBuffer(GL_ARRAY_BUFFER, graphics->text_vbo);
		glBufferData(
			GL_ARRAY_BUFFER, vertex_count * 8 * sizeof(float), vertices, GL_DYNAMIC_DRAW
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics->text_ebo);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			index_count * sizeof(unsigned int),
			indices,
			GL_DYNAMIC_DRAW
		);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(
			1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(2 * sizeof(float))
		);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(
			2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(4 * sizeof(float))
		);
		glEnableVertexAttribArray(2);

		glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}

	free(vertices);
	free(indices);
}

float graphics_measure_text(struct graphics *graphics, const char *text, int size)
{
	if (!graphics || !text) {
		return 0.0f;
	}

	if (!graphics->font_atlas) {
		return (float)(strlen(text) * size) * 0.6f;
	}

	return msdf_atlas_measure_text(graphics->font_atlas, text, (float)size);
}

bool graphics_poll_events(struct graphics *graphics)
{
	if (!graphics || !graphics->window) {
		return false;
	}

	return window_poll_events(graphics->window);
}

struct window *graphics_get_window(struct graphics *graphics)
{
	if (graphics == NULL) {
		return NULL;
	}

	return graphics->window;
}

float graphics_get_fps(const struct graphics *graphics)
{
	if (graphics == NULL) {
		return 0.0f;
	}

	return graphics->current_fps;
}
