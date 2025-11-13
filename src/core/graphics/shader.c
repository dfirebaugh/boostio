#include "shader.h"
#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Shader {
	unsigned int program_id;
	unsigned int vertex_shader;
	unsigned int fragment_shader;
};

struct Shader *shader_create(const char *vertex_source, const char *fragment_source)
{
	struct Shader *shader = (struct Shader *)malloc(sizeof(struct Shader));
	if (!shader) {
		return NULL;
	}

	memset(shader, 0, sizeof(struct Shader));

	shader->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader->vertex_shader, 1, &vertex_source, NULL);
	glCompileShader(shader->vertex_shader);

	int success;
	char info_log[512];
	glGetShaderiv(shader->vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader->vertex_shader, 512, NULL, info_log);
		fprintf(stderr, "Vertex shader compilation failed: %s\n", info_log);
		glDeleteShader(shader->vertex_shader);
		free(shader);
		return NULL;
	}

	shader->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader->fragment_shader, 1, &fragment_source, NULL);
	glCompileShader(shader->fragment_shader);

	glGetShaderiv(shader->fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader->fragment_shader, 512, NULL, info_log);
		fprintf(stderr, "Fragment shader compilation failed: %s\n", info_log);
		glDeleteShader(shader->vertex_shader);
		glDeleteShader(shader->fragment_shader);
		free(shader);
		return NULL;
	}

	shader->program_id = glCreateProgram();
	glAttachShader(shader->program_id, shader->vertex_shader);
	glAttachShader(shader->program_id, shader->fragment_shader);
	glLinkProgram(shader->program_id);

	glGetProgramiv(shader->program_id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader->program_id, 512, NULL, info_log);
		fprintf(stderr, "Shader linking failed: %s\n", info_log);
		glDeleteShader(shader->vertex_shader);
		glDeleteShader(shader->fragment_shader);
		glDeleteProgram(shader->program_id);
		free(shader);
		return NULL;
	}

	return shader;
}

void shader_destroy(struct Shader *shader)
{
	if (!shader) {
		return;
	}

	if (shader->program_id) {
		glDeleteProgram(shader->program_id);
	}

	if (shader->vertex_shader) {
		glDeleteShader(shader->vertex_shader);
	}

	if (shader->fragment_shader) {
		glDeleteShader(shader->fragment_shader);
	}

	free(shader);
}

void shader_use(const struct Shader *shader)
{
	if (shader) {
		glUseProgram(shader->program_id);
	}
}

void shader_set_mat4(const struct Shader *shader, const char *name, const float *matrix)
{
	if (!shader)
		return;
	int location = glGetUniformLocation(shader->program_id, name);
	if (location >= 0) {
		glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
	}
}

void shader_set_float(const struct Shader *shader, const char *name, float value)
{
	if (!shader)
		return;
	int location = glGetUniformLocation(shader->program_id, name);
	if (location >= 0) {
		glUniform1f(location, value);
	}
}

void shader_set_vec2(const struct Shader *shader, const char *name, float x, float y)
{
	if (!shader)
		return;
	int location = glGetUniformLocation(shader->program_id, name);
	if (location >= 0) {
		glUniform2f(location, x, y);
	}
}

void shader_set_int(const struct Shader *shader, const char *name, int value)
{
	if (!shader)
		return;
	int location = glGetUniformLocation(shader->program_id, name);
	if (location >= 0) {
		glUniform1i(location, value);
	}
}

void ortho_projection(
	float left, float right, float bottom, float top, float near, float far, float *out
)
{
	memset(out, 0, 16 * sizeof(float));
	out[0] = 2.0f / (right - left);
	out[5] = 2.0f / (top - bottom);
	out[10] = -2.0f / (far - near);
	out[12] = -(right + left) / (right - left);
	out[13] = -(top + bottom) / (top - bottom);
	out[14] = -(far + near) / (far - near);
	out[15] = 1.0f;
}
