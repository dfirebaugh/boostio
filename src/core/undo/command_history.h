#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include "synth.h"

#include <stdbool.h>
#include <stdint.h>

struct app_state;

#define MAX_UNDO_HISTORY 100

struct stored_note {
	uint32_t id;
	uint32_t ms;
	uint16_t duration_ms;
	uint8_t voice;
	uint8_t piano_key;
	float frequency;
	enum waveform_type waveform;
	uint8_t duty_cycle;
	int16_t decay;
	int8_t amplitude_dbfs;
	uint8_t nes_noise_period;
	bool nes_noise_mode_flag;
	uint16_t nes_noise_lfsr_init;
	bool restart_phase;
};

enum command_type {
	CMD_ADD_NOTE,
	CMD_DELETE_NOTE,
	CMD_MOVE_NOTE,
	CMD_RESIZE_NOTE,
	CMD_SET_NOTE_VOICE,
	CMD_SET_NOTE_INSTRUMENT,
	CMD_BATCH_START,
	CMD_BATCH_END,
};

struct add_note_data {
	struct stored_note note;
};

struct delete_note_data {
	struct stored_note note;
	uint32_t index;
};

struct move_note_data {
	uint32_t note_id;
	int32_t delta_ms;
	int32_t delta_piano_key;
};

struct resize_note_data {
	uint32_t note_id;
	int32_t delta_duration_ms;
	bool from_left;
	int32_t delta_ms;
};

struct set_note_voice_data {
	uint32_t note_id;
	uint8_t old_voice;
	uint8_t new_voice;
};

struct set_note_instrument_data {
	uint32_t note_id;
	enum waveform_type old_waveform;
	uint8_t old_duty_cycle;
	int16_t old_decay;
	int8_t old_amplitude_dbfs;
	bool old_nes_noise_mode_flag;
	uint16_t old_nes_noise_lfsr_init;
	uint8_t new_instrument_index;
};

union command_data {
	struct add_note_data add_note;
	struct delete_note_data delete_note;
	struct move_note_data move_note;
	struct resize_note_data resize_note;
	struct set_note_voice_data set_note_voice;
	struct set_note_instrument_data set_note_instrument;
};

struct command {
	enum command_type type;
	union command_data data;
};

struct command_history {
	struct command undo_stack[MAX_UNDO_HISTORY];
	uint32_t undo_count;
	struct command redo_stack[MAX_UNDO_HISTORY];
	uint32_t redo_count;
	bool in_batch;
	uint32_t batch_start_index;
};

void command_history_init(struct command_history *history);

void command_history_push(struct command_history *history, struct command cmd);

bool command_history_undo(struct command_history *history, struct app_state *state, void *audio);

bool command_history_redo(struct command_history *history, struct app_state *state, void *audio);

void command_history_clear(struct command_history *history);

void command_history_begin_batch(struct command_history *history);

void command_history_end_batch(struct command_history *history);

#endif
