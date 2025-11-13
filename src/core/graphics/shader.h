#ifndef BOOSTIO_SHADER_H
#define BOOSTIO_SHADER_H

struct shader;

struct shader *shader_create(const char *vertex_source, const char *fragment_source);
void shader_destroy(struct shader *shader);

void shader_use(const struct shader *shader);
void shader_set_mat4(const struct shader *shader, const char *name, const float *matrix);
void shader_set_float(const struct shader *shader, const char *name, float value);
void shader_set_vec2(const struct shader *shader, const char *name, float x, float y);
void shader_set_int(const struct shader *shader, const char *name, int value);

void ortho_projection(
	float left, float right, float bottom, float top, float near, float far, float *out
);

#endif
