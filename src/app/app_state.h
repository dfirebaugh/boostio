#ifndef BOOSTIO_APP_STATE_H
#define BOOSTIO_APP_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "core/audio/scale.h"
#include "core/audio/synth.h"
#include "core/theme/theme.h"

struct Sequencer;

#define UI_MAX_NOTES 1024
#define UI_MAX_SELECTION 256
#define MAX_INSTRUMENTS 16

struct instrument {
	char name[32];
	enum WaveformType waveform;
	uint8_t duty_cycle;
	int8_t amplitude_dbfs;
	int16_t decay;
	uint16_t default_duration_ms;
	uint8_t color_r;
	uint8_t color_g;
	uint8_t color_b;
	bool nes_noise_mode_flag;
	uint16_t nes_noise_lfsr;
};

struct viewport {
	float time_offset;
	int32_t note_offset;
	float pixels_per_ms;
	float piano_key_height;
	float grid_x;
	float grid_y;
	float grid_width;
	float grid_height;
};

struct ui_note {
	uint32_t id;
	uint32_t ms;
	uint16_t duration_ms;
	uint8_t voice;
	uint8_t piano_key;
};

struct selection {
	uint32_t selected_ids[UI_MAX_SELECTION];
	uint32_t count;
};

struct app_state {
	int window_width;
	int window_height;

	float mouse_x;
	float mouse_y;

	bool playing;
	uint32_t playhead_ms;

	uint32_t bpm;
	bool snap_enabled;

	uint8_t selected_voice;

	struct instrument instruments[MAX_INSTRUMENTS];
	uint8_t instrument_count;
	uint8_t selected_instrument;

	enum scale_type selected_scale;
	enum root_note selected_root;
	bool show_scale_highlights;
	bool fold_mode;

	bool voice_hidden[8];
	bool voice_solo[8];
	bool voice_muted[8];

	bool show_help;
	bool show_fps;

	struct theme theme;

	struct viewport viewport;
	struct ui_note notes[UI_MAX_NOTES];
	uint32_t note_count;
	struct selection selection;
	uint32_t next_note_id;

	char current_file_path[512];
};

void app_state_init(struct app_state *state);

void app_state_update_dimensions(struct app_state *state, int width, int height);

void app_state_update_mouse(struct app_state *state, float x, float y);

void app_state_set_bpm(struct app_state *state, uint32_t bpm);

void app_state_scroll_horizontal(struct app_state *state, float delta_ms);

void app_state_scroll_vertical(struct app_state *state, int delta_keys);

void app_state_zoom_horizontal(struct app_state *state, float factor);

void app_state_zoom_vertical(struct app_state *state, float factor);

void app_state_zoom_horizontal_at_mouse(struct app_state *state, float factor, float mouse_x);

void app_state_zoom_vertical_at_mouse(struct app_state *state, float factor, float mouse_y);

void app_state_sync_notes_from_sequencer(
	struct app_state *state, const struct Sequencer *sequencer
);

#endif
