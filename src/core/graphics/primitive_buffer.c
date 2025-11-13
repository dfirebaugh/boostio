#include "color.h"
#include <glad/gl.h>
#include "primitive_buffer.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *primitive_vertex_shader = "#version 120\n"
											 "attribute vec2 in_pos;\n"
											 "attribute vec2 in_local_pos;\n"
											 "attribute float in_op_code;\n"
											 "attribute float in_radius;\n"
											 "attribute vec4 in_color;\n"
											 "attribute float in_width;\n"
											 "attribute float in_height;\n"
											 "attribute float in_outline_width;\n"
											 "attribute vec4 in_outline_color;\n"
											 "\n"
											 "varying vec2 local_pos;\n"
											 "varying float op_code;\n"
											 "varying float radius;\n"
											 "varying vec4 color;\n"
											 "varying float width;\n"
											 "varying float height;\n"
											 "varying float outline_width;\n"
											 "varying vec4 outline_color;\n"
											 "\n"
											 "void main() {\n"
											 "    gl_Position = vec4(in_pos, 0.0, 1.0);\n"
											 "    local_pos = in_local_pos;\n"
											 "    op_code = in_op_code;\n"
											 "    radius = in_radius;\n"
											 "    color = in_color;\n"
											 "    width = in_width;\n"
											 "    height = in_height;\n"
											 "    outline_width = in_outline_width;\n"
											 "    outline_color = in_outline_color;\n"
											 "}\n";

static const char *primitive_fragment_shader =
		"#version 120\n"
		"varying vec2 local_pos;\n"
		"varying float op_code;\n"
		"varying float radius;\n"
		"varying vec4 color;\n"
		"varying float width;\n"
		"varying float height;\n"
		"varying float outline_width;\n"
		"varying vec4 outline_color;\n"
		"\n"
		"const float OP_CODE_RECT = 3.0;\n"
		"\n"
		"float sdRoundedRect(vec2 p, vec2 bounds, float r) {\n"
		"    vec2 b = bounds - vec2(r);\n"
		"    vec2 q = abs(p) - b;\n"
		"    return length(max(q, 0.0)) - r;\n"
		"}\n"
		"\n"
		"float sdRect(vec2 p, vec2 bounds) {\n"
		"    vec2 q = abs(p) - bounds;\n"
		"    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);\n"
		"}\n"
		"\n"
		"void main() {\n"
		"    vec2 p = local_pos;\n"
		"    gl_FragColor = vec4(0.0);\n"
		"\n"
		"    if (op_code == OP_CODE_RECT) {\n"
		"        float sdf;\n"
		"        if (radius <= 0.0) {\n"
		"            sdf = sdRect(p, vec2(width * 0.5, height * 0.5));\n"
		"        } else {\n"
		"            sdf = sdRoundedRect(p, vec2(width * 0.5, height * 0.5), radius);\n"
		"        }\n"
		"        if (sdf < 0.0) {\n"
		"            gl_FragColor = color;\n"
		"        } else if (outline_width > 0.0 && sdf < outline_width) {\n"
		"            gl_FragColor = outline_color;\n"
		"        }\n"
		"    }\n"
		"}\n";

struct primitive_vertex
{
	float pos[2];
	float local_pos[2];
	float op_code;
	float radius;
	float color[4];
	float width;
	float height;
	float outline_width;
	float outline_color[4];
};

struct PrimitiveBuffer
{
	unsigned int shader_program;
	unsigned int vao;
	unsigned int vbo;

	struct primitive_vertex *vertices;
	int vertex_count;
	int vertex_capacity;
};

struct PrimitiveBuffer *primitive_buffer_create(void)
{
	struct PrimitiveBuffer *buffer =
			(struct PrimitiveBuffer *)malloc(sizeof(struct PrimitiveBuffer));
	if (!buffer)
	{
		return NULL;
	}

	memset(buffer, 0, sizeof(struct PrimitiveBuffer));

	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &primitive_vertex_shader, NULL);
	glCompileShader(vertex_shader);

	int success;
	char info_log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		fprintf(stderr, "Primitive vertex shader failed: %s\n", info_log);
		glDeleteShader(vertex_shader);
		free(buffer);
		return NULL;
	}

	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &primitive_fragment_shader, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		fprintf(stderr, "Primitive fragment shader failed: %s\n", info_log);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		free(buffer);
		return NULL;
	}

	buffer->shader_program = glCreateProgram();
	glAttachShader(buffer->shader_program, vertex_shader);
	glAttachShader(buffer->shader_program, fragment_shader);

	glBindAttribLocation(buffer->shader_program, 0, "in_pos");
	glBindAttribLocation(buffer->shader_program, 1, "in_local_pos");
	glBindAttribLocation(buffer->shader_program, 2, "in_op_code");
	glBindAttribLocation(buffer->shader_program, 3, "in_radius");
	glBindAttribLocation(buffer->shader_program, 4, "in_color");
	glBindAttribLocation(buffer->shader_program, 5, "in_width");
	glBindAttribLocation(buffer->shader_program, 6, "in_height");
	glBindAttribLocation(buffer->shader_program, 7, "in_outline_width");
	glBindAttribLocation(buffer->shader_program, 8, "in_outline_color");

	glLinkProgram(buffer->shader_program);

	glGetProgramiv(buffer->shader_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(buffer->shader_program, 512, NULL, info_log);
		fprintf(stderr, "Primitive shader linking failed: %s\n", info_log);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteProgram(buffer->shader_program);
		free(buffer);
		return NULL;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glGenVertexArrays(1, &buffer->vao);
	glGenBuffers(1, &buffer->vbo);

	buffer->vertex_capacity = 1024;
	buffer->vertices = (struct primitive_vertex *)malloc(sizeof(struct primitive_vertex) * buffer->vertex_capacity);

	return buffer;
}

void primitive_buffer_destroy(struct PrimitiveBuffer *buffer)
{
	if (!buffer)
	{
		return;
	}

	if (buffer->vao)
	{
		glDeleteVertexArrays(1, &buffer->vao);
	}

	if (buffer->vbo)
	{
		glDeleteBuffers(1, &buffer->vbo);
	}

	if (buffer->shader_program)
	{
		glDeleteProgram(buffer->shader_program);
	}

	if (buffer->vertices)
	{
		free(buffer->vertices);
	}

	free(buffer);
}

void primitive_buffer_begin(struct PrimitiveBuffer *buffer)
{
	if (!buffer)
	{
		return;
	}

	buffer->vertex_count = 0;
}

void primitive_buffer_add_rect(
		struct PrimitiveBuffer *buffer, float x, float y, float width, float height,
		struct Color color, float radius, float outline_width, struct Color outline_color
)
{
	if (!buffer)
	{
		return;
	}

	if (buffer->vertex_count + 6 > buffer->vertex_capacity)
	{
		buffer->vertex_capacity *= 2;
		buffer->vertices = (struct primitive_vertex *)realloc(
				buffer->vertices, sizeof(struct primitive_vertex) * buffer->vertex_capacity
		);
	}

	float r = color.r / 255.0f;
	float g = color.g / 255.0f;
	float b = color.b / 255.0f;
	float a = color.a / 255.0f;

	float or = outline_color.r / 255.0f;
	float og = outline_color.g / 255.0f;
	float ob = outline_color.b / 255.0f;
	float oa = outline_color.a / 255.0f;

	float expand = outline_width;
	float quad_x = x - expand;
	float quad_y = y - expand;
	float quad_width = width + expand * 2.0f;
	float quad_height = height + expand * 2.0f;

	float center_x = x + width / 2.0f;
	float center_y = y + height / 2.0f;

	struct primitive_vertex *v = &buffer->vertices[buffer->vertex_count];

	v[0].pos[0] = quad_x;
	v[0].pos[1] = quad_y;
	v[0].local_pos[0] = -width / 2.0f - expand;
	v[0].local_pos[1] = -height / 2.0f - expand;
	v[0].op_code = 3.0f;
	v[0].radius = radius;
	v[0].color[0] = r;
	v[0].color[1] = g;
	v[0].color[2] = b;
	v[0].color[3] = a;
	v[0].width = width;
	v[0].height = height;
	v[0].outline_width = outline_width;
	v[0].outline_color[0] = or ;
	v[0].outline_color[1] = og;
	v[0].outline_color[2] = ob;
	v[0].outline_color[3] = oa;

	v[1].pos[0] = quad_x + quad_width;
	v[1].pos[1] = quad_y;
	v[1].local_pos[0] = width / 2.0f + expand;
	v[1].local_pos[1] = -height / 2.0f - expand;
	v[1].op_code = 3.0f;
	v[1].radius = radius;
	v[1].color[0] = r;
	v[1].color[1] = g;
	v[1].color[2] = b;
	v[1].color[3] = a;
	v[1].width = width;
	v[1].height = height;
	v[1].outline_width = outline_width;
	v[1].outline_color[0] = or ;
	v[1].outline_color[1] = og;
	v[1].outline_color[2] = ob;
	v[1].outline_color[3] = oa;

	v[2].pos[0] = quad_x + quad_width;
	v[2].pos[1] = quad_y + quad_height;
	v[2].local_pos[0] = width / 2.0f + expand;
	v[2].local_pos[1] = height / 2.0f + expand;
	v[2].op_code = 3.0f;
	v[2].radius = radius;
	v[2].color[0] = r;
	v[2].color[1] = g;
	v[2].color[2] = b;
	v[2].color[3] = a;
	v[2].width = width;
	v[2].height = height;
	v[2].outline_width = outline_width;
	v[2].outline_color[0] = or ;
	v[2].outline_color[1] = og;
	v[2].outline_color[2] = ob;
	v[2].outline_color[3] = oa;

	v[3].pos[0] = quad_x;
	v[3].pos[1] = quad_y;
	v[3].local_pos[0] = -width / 2.0f - expand;
	v[3].local_pos[1] = -height / 2.0f - expand;
	v[3].op_code = 3.0f;
	v[3].radius = radius;
	v[3].color[0] = r;
	v[3].color[1] = g;
	v[3].color[2] = b;
	v[3].color[3] = a;
	v[3].width = width;
	v[3].height = height;
	v[3].outline_width = outline_width;
	v[3].outline_color[0] = or ;
	v[3].outline_color[1] = og;
	v[3].outline_color[2] = ob;
	v[3].outline_color[3] = oa;

	v[4].pos[0] = quad_x + quad_width;
	v[4].pos[1] = quad_y + quad_height;
	v[4].local_pos[0] = width / 2.0f + expand;
	v[4].local_pos[1] = height / 2.0f + expand;
	v[4].op_code = 3.0f;
	v[4].radius = radius;
	v[4].color[0] = r;
	v[4].color[1] = g;
	v[4].color[2] = b;
	v[4].color[3] = a;
	v[4].width = width;
	v[4].height = height;
	v[4].outline_width = outline_width;
	v[4].outline_color[0] = or ;
	v[4].outline_color[1] = og;
	v[4].outline_color[2] = ob;
	v[4].outline_color[3] = oa;

	v[5].pos[0] = quad_x;
	v[5].pos[1] = quad_y + quad_height;
	v[5].local_pos[0] = -width / 2.0f - expand;
	v[5].local_pos[1] = height / 2.0f + expand;
	v[5].op_code = 3.0f;
	v[5].radius = radius;
	v[5].color[0] = r;
	v[5].color[1] = g;
	v[5].color[2] = b;
	v[5].color[3] = a;
	v[5].width = width;
	v[5].height = height;
	v[5].outline_width = outline_width;
	v[5].outline_color[0] = or ;
	v[5].outline_color[1] = og;
	v[5].outline_color[2] = ob;
	v[5].outline_color[3] = oa;

	buffer->vertex_count += 6;
}

void primitive_buffer_add_line(
		struct PrimitiveBuffer *buffer, float x1, float y1, float x2, float y2, struct Color color
)
{
	struct Color no_outline = {0, 0, 0, 0};

	if (fabsf(y2 - y1) < 0.5f)
	{
		float width = fabsf(x2 - x1);
		float x = fminf(x1, x2);
		primitive_buffer_add_rect(buffer, x, y1, width, 1.0f, color, 0.0f, 0.0f, no_outline);
	}
	else if (fabsf(x2 - x1) < 0.5f)
	{
		float height = fabsf(y2 - y1);
		float y = fminf(y1, y2);
		primitive_buffer_add_rect(buffer, x1, y, 1.0f, height, color, 0.0f, 0.0f, no_outline);
	}
	else
	{
		float dx = x2 - x1;
		float dy = y2 - y1;
		float len = sqrtf(dx * dx + dy * dy);

		if (len < 0.001f)
		{
			return;
		}

		float thickness = 1.0f;
		float nx = -dy / len * thickness / 2.0f;
		float ny = dx / len * thickness / 2.0f;

		primitive_buffer_add_rect(buffer, x1 + nx, y1 + ny, len, thickness, color, 0.0f, 0.0f, no_outline);
	}
}

void primitive_buffer_render(struct PrimitiveBuffer *buffer, int window_width, int window_height)
{
	if (!buffer || buffer->vertex_count == 0)
	{
		return;
	}

	for (int i = 0; i < buffer->vertex_count; i++)
	{
		buffer->vertices[i].pos[0] = (buffer->vertices[i].pos[0] / window_width) * 2.0f - 1.0f;
		buffer->vertices[i].pos[1] = 1.0f - (buffer->vertices[i].pos[1] / window_height) * 2.0f;
	}

	glUseProgram(buffer->shader_program);
	glBindVertexArray(buffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
	glBufferData(
			GL_ARRAY_BUFFER, buffer->vertex_count * sizeof(struct primitive_vertex), buffer->vertices,
			GL_DYNAMIC_DRAW
	);

	size_t stride = sizeof(struct primitive_vertex);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
			1, 2, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, local_pos)
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
			2, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, op_code)
	);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
			3, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, radius)
	);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(
			4, 4, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, color)
	);
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(
			5, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, width)
	);
	glEnableVertexAttribArray(5);

	glVertexAttribPointer(
			6, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, height)
	);
	glEnableVertexAttribArray(6);

	glVertexAttribPointer(
			7, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, outline_width)
	);
	glEnableVertexAttribArray(7);

	glVertexAttribPointer(
			8, 4, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(struct primitive_vertex, outline_color)
	);
	glEnableVertexAttribArray(8);

	glDrawArrays(GL_TRIANGLES, 0, buffer->vertex_count);

	glBindVertexArray(0);
}
