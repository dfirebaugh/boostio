#ifndef BOOSTIO_APP_STATE_H
#define BOOSTIO_APP_STATE_H

#include <stdbool.h>
#include <stdint.h>

struct app_state
{
	int window_width;
	int window_height;

	float mouse_x;
	float mouse_y;

	bool playing;
	uint32_t playhead_ms;

	uint32_t bpm;
	bool snap_enabled;

	uint8_t selected_voice;

	bool show_help;
	bool show_fps;
};

void app_state_init(struct app_state *state);

void app_state_update_dimensions(struct app_state *state, int width, int height);

void app_state_update_mouse(struct app_state *state, float x, float y);

void app_state_set_bpm(struct app_state *state, uint32_t bpm);

#endif
