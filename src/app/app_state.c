#include <string.h>

#include "app_state.h"

void app_state_init(struct app_state *state)
{
	if (state == NULL)
	{
		return;
	}

	memset(state, 0, sizeof(struct app_state));

	state->window_width = 800;
	state->window_height = 600;
	state->bpm = 120;
	state->snap_enabled = true;
	state->selected_voice = 0;
}

void app_state_update_dimensions(struct app_state *state, int width, int height)
{
	if (state == NULL)
	{
		return;
	}

	state->window_width = width;
	state->window_height = height;
}

void app_state_update_mouse(struct app_state *state, float x, float y)
{
	if (state == NULL)
	{
		return;
	}

	state->mouse_x = x;
	state->mouse_y = y;
}

void app_state_set_bpm(struct app_state *state, uint32_t bpm)
{
	if (state == NULL || bpm == 0)
	{
		return;
	}

	state->bpm = bpm;
}
