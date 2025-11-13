#include <string.h>

#include "app_state.h"
#include "sequencer.h"

void app_state_init(struct app_state *state)
{
	if (state == NULL) {
		return;
	}

	memset(state, 0, sizeof(struct app_state));

	command_history_init(&state->history);

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
	state->viewport.note_offset = 15;
	state->viewport.pixels_per_ms = 0.2f;
	state->viewport.piano_key_height = 20.0f;
	state->viewport.grid_x = 120.0f;
	state->viewport.grid_y = 90.0f;
	state->viewport.grid_width = 680.0f;
	state->viewport.grid_height = 480.0f;

	state->note_count = 0;
	state->selection.count = 0;
	state->next_note_id = 1;

	state->instrument_count = 6;
	state->selected_instrument = 0;

	strcpy(state->instruments[0].name, "Square");
	state->instruments[0].waveform = WAVEFORM_SQUARE;
	state->instruments[0].duty_cycle = 128;
	state->instruments[0].amplitude_dbfs = -6;
	state->instruments[0].decay = 0;
	state->instruments[0].default_duration_ms = 100;
	state->instruments[0].color_r = 78;
	state->instruments[0].color_g = 205;
	state->instruments[0].color_b = 196;
	state->instruments[0].nes_noise_mode_flag = false;
	state->instruments[0].nes_noise_lfsr = 1;

	strcpy(state->instruments[1].name, "Triangle");
	state->instruments[1].waveform = WAVEFORM_TRIANGLE;
	state->instruments[1].duty_cycle = 128;
	state->instruments[1].amplitude_dbfs = -6;
	state->instruments[1].decay = 0;
	state->instruments[1].default_duration_ms = 100;
	state->instruments[1].color_r = 69;
	state->instruments[1].color_g = 170;
	state->instruments[1].color_b = 242;
	state->instruments[1].nes_noise_mode_flag = false;
	state->instruments[1].nes_noise_lfsr = 1;

	strcpy(state->instruments[2].name, "Sawtooth");
	state->instruments[2].waveform = WAVEFORM_SAWTOOTH;
	state->instruments[2].duty_cycle = 128;
	state->instruments[2].amplitude_dbfs = -6;
	state->instruments[2].decay = 0;
	state->instruments[2].default_duration_ms = 100;
	state->instruments[2].color_r = 253;
	state->instruments[2].color_g = 150;
	state->instruments[2].color_b = 68;
	state->instruments[2].nes_noise_mode_flag = false;
	state->instruments[2].nes_noise_lfsr = 1;

	strcpy(state->instruments[3].name, "Sine");
	state->instruments[3].waveform = WAVEFORM_SINE;
	state->instruments[3].duty_cycle = 128;
	state->instruments[3].amplitude_dbfs = -6;
	state->instruments[3].decay = 0;
	state->instruments[3].default_duration_ms = 100;
	state->instruments[3].color_r = 165;
	state->instruments[3].color_g = 94;
	state->instruments[3].color_b = 234;
	state->instruments[3].nes_noise_mode_flag = false;
	state->instruments[3].nes_noise_lfsr = 1;

	strcpy(state->instruments[4].name, "NES Noise");
	state->instruments[4].waveform = WAVEFORM_NES_NOISE;
	state->instruments[4].duty_cycle = 15;
	state->instruments[4].amplitude_dbfs = -6;
	state->instruments[4].decay = 200;
	state->instruments[4].default_duration_ms = 100;
	state->instruments[4].color_r = 231;
	state->instruments[4].color_g = 76;
	state->instruments[4].color_b = 60;
	state->instruments[4].nes_noise_mode_flag = true;
	state->instruments[4].nes_noise_lfsr = 1;

	strcpy(state->instruments[5].name, "NES Metallic");
	state->instruments[5].waveform = WAVEFORM_NES_NOISE;
	state->instruments[5].duty_cycle = 15;
	state->instruments[5].amplitude_dbfs = -6;
	state->instruments[5].decay = 50;
	state->instruments[5].default_duration_ms = 200;
	state->instruments[5].color_r = 230;
	state->instruments[5].color_g = 179;
	state->instruments[5].color_b = 51;
	state->instruments[5].nes_noise_mode_flag = false;
	state->instruments[5].nes_noise_lfsr = 1;
}

void app_state_update_dimensions(struct app_state *state, int width, int height)
{
	if (state == NULL) {
		return;
	}

	state->window_width = width;
	state->window_height = height;

	state->viewport.grid_width = (float)width - state->viewport.grid_x;
	state->viewport.grid_height = (float)height - state->viewport.grid_y - 30.0f;
}

void app_state_update_mouse(struct app_state *state, float x, float y)
{
	if (state == NULL) {
		return;
	}

	state->mouse_x = x;
	state->mouse_y = y;
}

void app_state_set_bpm(struct app_state *state, uint32_t bpm)
{
	if (state == NULL || bpm == 0) {
		return;
	}

	state->bpm = bpm;
}

void app_state_scroll_horizontal(struct app_state *state, float delta_ms)
{
	if (state == NULL) {
		return;
	}

	state->viewport.time_offset += delta_ms;
	if (state->viewport.time_offset < 0.0f) {
		state->viewport.time_offset = 0.0f;
	}
}

void app_state_scroll_vertical(struct app_state *state, int delta_keys)
{
	if (state == NULL) {
		return;
	}

	state->viewport.note_offset += delta_keys;
	if (state->viewport.note_offset < 0) {
		state->viewport.note_offset = 0;
	}
	if (state->viewport.note_offset > 54) {
		state->viewport.note_offset = 54;
	}
}

void app_state_zoom_horizontal(struct app_state *state, float factor)
{
	if (state == NULL) {
		return;
	}

	state->viewport.pixels_per_ms *= factor;

	if (state->viewport.pixels_per_ms < 0.05f) {
		state->viewport.pixels_per_ms = 0.05f;
	}
	if (state->viewport.pixels_per_ms > 2.0f) {
		state->viewport.pixels_per_ms = 2.0f;
	}
}

void app_state_zoom_vertical(struct app_state *state, float factor)
{
	if (state == NULL) {
		return;
	}

	state->viewport.piano_key_height *= factor;

	if (state->viewport.piano_key_height < 8.0f) {
		state->viewport.piano_key_height = 8.0f;
	}
	if (state->viewport.piano_key_height > 60.0f) {
		state->viewport.piano_key_height = 60.0f;
	}
}

void app_state_zoom_horizontal_at_mouse(struct app_state *state, float factor, float mouse_x)
{
	if (state == NULL) {
		return;
	}

	if (mouse_x < state->viewport.grid_x ||
	    mouse_x > state->viewport.grid_x + state->viewport.grid_width) {
		app_state_zoom_horizontal(state, factor);
		return;
	}

	float old_pixels_per_ms = state->viewport.pixels_per_ms;
	float relative_x = mouse_x - state->viewport.grid_x;
	float mouse_time_ms = state->viewport.time_offset + relative_x / old_pixels_per_ms;

	state->viewport.pixels_per_ms *= factor;

	if (state->viewport.pixels_per_ms < 0.05f) {
		state->viewport.pixels_per_ms = 0.05f;
	}
	if (state->viewport.pixels_per_ms > 2.0f) {
		state->viewport.pixels_per_ms = 2.0f;
	}

	state->viewport.time_offset = mouse_time_ms - relative_x / state->viewport.pixels_per_ms;
	if (state->viewport.time_offset < 0.0f) {
		state->viewport.time_offset = 0.0f;
	}
}

void app_state_zoom_vertical_at_mouse(struct app_state *state, float factor, float mouse_y)
{
	if (state == NULL) {
		return;
	}

	if (mouse_y < state->viewport.grid_y ||
	    mouse_y > state->viewport.grid_y + state->viewport.grid_height) {
		app_state_zoom_vertical(state, factor);
		return;
	}

	float old_piano_key_height = state->viewport.piano_key_height;
	float relative_y = mouse_y - state->viewport.grid_y;
	float mouse_row = relative_y / old_piano_key_height;
	int32_t old_note_offset = state->viewport.note_offset;

	state->viewport.piano_key_height *= factor;

	if (state->viewport.piano_key_height < 8.0f) {
		state->viewport.piano_key_height = 8.0f;
	}
	if (state->viewport.piano_key_height > 60.0f) {
		state->viewport.piano_key_height = 60.0f;
	}

	float new_row = relative_y / state->viewport.piano_key_height;
	float row_delta = mouse_row - new_row;
	state->viewport.note_offset = old_note_offset + (int32_t)(row_delta + 0.5f);

	if (state->viewport.note_offset < 0) {
		state->viewport.note_offset = 0;
	}
	if (state->viewport.note_offset > 54) {
		state->viewport.note_offset = 54;
	}
}

void app_state_sync_notes_from_sequencer(struct app_state *state, const struct sequencer *sequencer)
{
	if (state == NULL || sequencer == NULL) {
		return;
	}

	state->note_count = 0;

	uint32_t count = sequencer->note_count;
	if (count > UI_MAX_NOTES) {
		count = UI_MAX_NOTES;
	}

	for (uint32_t i = 0; i < count; i++) {
		const struct note *seq_note = &sequencer->notes[i];
		struct ui_note *ui_note = &state->notes[i];

		ui_note->id = state->next_note_id++;
		ui_note->ms = seq_note->time_ms;
		ui_note->duration_ms = (uint16_t)seq_note->params.duration_ms;
		ui_note->voice = seq_note->params.voice_index >= 0
					 ? (uint8_t)seq_note->params.voice_index
					 : 0;
		ui_note->piano_key = seq_note->params.piano_key;
		ui_note->frequency = seq_note->params.frequency;
		ui_note->waveform = seq_note->params.waveform;
		ui_note->duty_cycle = seq_note->params.duty_cycle;
		ui_note->decay = seq_note->params.decay;
		ui_note->amplitude_dbfs = seq_note->params.amplitude_dbfs;
		ui_note->nes_noise_period = seq_note->params.nes_noise_period;
		ui_note->nes_noise_mode_flag = seq_note->params.nes_noise_mode_flag;
		ui_note->nes_noise_lfsr_init = seq_note->params.nes_noise_lfsr_init;
		ui_note->restart_phase = seq_note->params.restart_phase;
	}

	state->note_count = count;
}

void app_state_sync_notes_to_sequencer(
	const struct app_state *state, struct sequencer *sequencer, struct audio *audio
)
{
	if (state == NULL || sequencer == NULL || audio == NULL) {
		return;
	}

	sequencer_clear_notes(sequencer);

	for (uint32_t i = 0; i < state->note_count; i++) {
		const struct ui_note *ui_note = &state->notes[i];

		struct note_params params = {
			.frequency = ui_note->frequency,
			.duration_ms = (float)ui_note->duration_ms,
			.waveform = ui_note->waveform,
			.duty_cycle = ui_note->duty_cycle,
			.decay = ui_note->decay,
			.amplitude_dbfs = ui_note->amplitude_dbfs,
			.nes_noise_period = ui_note->nes_noise_period,
			.nes_noise_mode_flag = ui_note->nes_noise_mode_flag,
			.nes_noise_lfsr_init = ui_note->nes_noise_lfsr_init,
			.restart_phase = ui_note->restart_phase,
			.voice_index = ui_note->voice,
			.piano_key = ui_note->piano_key
		};

		sequencer_add_note(sequencer, ui_note->ms, params);
	}
}
