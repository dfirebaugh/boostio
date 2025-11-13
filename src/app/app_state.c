#include <string.h>

#include "app_state.h"
#include "core/audio/sequencer.h"

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

	state->selected_scale = SCALE_PENTATONIC_MINOR;
	state->selected_root = ROOT_C;
	state->show_scale_highlights = true;
	state->fold_mode = false;

	theme_init_default(&state->theme);

	state->viewport.time_offset = 0.0f;
	state->viewport.note_offset = 60;
	state->viewport.pixels_per_ms = 0.2f;
	state->viewport.piano_key_height = 20.0f;
	state->viewport.grid_x = 120.0f;
	state->viewport.grid_y = 90.0f;
	state->viewport.grid_width = 680.0f;
	state->viewport.grid_height = 480.0f;

	state->note_count = 0;
	state->selection.count = 0;
	state->next_note_id = 1;
}

void app_state_update_dimensions(struct app_state *state, int width, int height)
{
	if (state == NULL)
	{
		return;
	}

	state->window_width = width;
	state->window_height = height;

	state->viewport.grid_width = (float)width - state->viewport.grid_x;
	state->viewport.grid_height = (float)height - state->viewport.grid_y - 30.0f;
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

void app_state_scroll_horizontal(struct app_state *state, float delta_ms)
{
	if (state == NULL)
	{
		return;
	}

	state->viewport.time_offset += delta_ms;
	if (state->viewport.time_offset < 0.0f)
	{
		state->viewport.time_offset = 0.0f;
	}
}

void app_state_scroll_vertical(struct app_state *state, int delta_keys)
{
	if (state == NULL)
	{
		return;
	}

	state->viewport.note_offset += delta_keys;
	if (state->viewport.note_offset < 0)
	{
		state->viewport.note_offset = 0;
	}
	if (state->viewport.note_offset > 127)
	{
		state->viewport.note_offset = 127;
	}
}

void app_state_zoom_horizontal(struct app_state *state, float factor)
{
	if (state == NULL)
	{
		return;
	}

	state->viewport.pixels_per_ms *= factor;

	if (state->viewport.pixels_per_ms < 0.05f)
	{
		state->viewport.pixels_per_ms = 0.05f;
	}
	if (state->viewport.pixels_per_ms > 2.0f)
	{
		state->viewport.pixels_per_ms = 2.0f;
	}
}

void app_state_zoom_vertical(struct app_state *state, float factor)
{
	if (state == NULL)
	{
		return;
	}

	state->viewport.piano_key_height *= factor;

	if (state->viewport.piano_key_height < 8.0f)
	{
		state->viewport.piano_key_height = 8.0f;
	}
	if (state->viewport.piano_key_height > 60.0f)
	{
		state->viewport.piano_key_height = 60.0f;
	}
}

void app_state_sync_notes_from_sequencer(
		struct app_state *state, const struct Sequencer *sequencer
)
{
	if (state == NULL || sequencer == NULL)
	{
		return;
	}

	state->note_count = 0;

	uint32_t count = sequencer->note_count;
	if (count > UI_MAX_NOTES)
	{
		count = UI_MAX_NOTES;
	}

	for (uint32_t i = 0; i < count; i++)
	{
		const struct Note *seq_note = &sequencer->notes[i];
		struct ui_note *ui_note = &state->notes[i];

		ui_note->id = state->next_note_id++;
		ui_note->ms = seq_note->time_ms;
		ui_note->duration_ms = (uint16_t)seq_note->params.duration_ms;
		ui_note->voice = seq_note->params.voice_index >= 0 ? (uint8_t)seq_note->params.voice_index : 0;
		ui_note->piano_key = seq_note->params.piano_key;
	}

	state->note_count = count;
}
