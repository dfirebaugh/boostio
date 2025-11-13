#include "command_history.h"
#include "app_state.h"
#include "audio.h"
#include "sequencer.h"

#include <string.h>

static void ui_note_to_stored(const struct ui_note *ui, struct stored_note *stored)
{
	stored->id = ui->id;
	stored->ms = ui->ms;
	stored->duration_ms = ui->duration_ms;
	stored->voice = ui->voice;
	stored->piano_key = ui->piano_key;
	stored->waveform = ui->waveform;
	stored->duty_cycle = ui->duty_cycle;
	stored->decay = ui->decay;
	stored->amplitude_dbfs = ui->amplitude_dbfs;
	stored->nes_noise_period = ui->nes_noise_period;
	stored->nes_noise_mode_flag = ui->nes_noise_mode_flag;
	stored->nes_noise_lfsr_init = ui->nes_noise_lfsr_init;
	stored->restart_phase = ui->restart_phase;
}

static void stored_to_ui_note(const struct stored_note *stored, struct ui_note *ui)
{
	ui->id = stored->id;
	ui->ms = stored->ms;
	ui->duration_ms = stored->duration_ms;
	ui->voice = stored->voice;
	ui->piano_key = stored->piano_key;
	ui->waveform = stored->waveform;
	ui->duty_cycle = stored->duty_cycle;
	ui->decay = stored->decay;
	ui->amplitude_dbfs = stored->amplitude_dbfs;
	ui->nes_noise_period = stored->nes_noise_period;
	ui->nes_noise_mode_flag = stored->nes_noise_mode_flag;
	ui->nes_noise_lfsr_init = stored->nes_noise_lfsr_init;
	ui->restart_phase = stored->restart_phase;
}

void command_history_init(struct command_history *history)
{
	if (history == NULL)
		return;

	history->undo_count = 0;
	history->redo_count = 0;
	history->in_batch = false;
	history->batch_start_index = 0;
}

void command_history_push(struct command_history *history, struct command cmd)
{
	if (history == NULL)
		return;

	if (history->undo_count >= MAX_UNDO_HISTORY) {
		for (uint32_t i = 0; i < MAX_UNDO_HISTORY - 1; i++) {
			history->undo_stack[i] = history->undo_stack[i + 1];
		}
		history->undo_count = MAX_UNDO_HISTORY - 1;
	}

	history->undo_stack[history->undo_count] = cmd;
	history->undo_count++;

	history->redo_count = 0;
}

static bool undo_add_note(struct app_state *state, struct add_note_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note.id) {
			for (uint32_t j = i; j < state->note_count - 1; j++) {
				state->notes[j] = state->notes[j + 1];
			}
			state->note_count--;

			for (uint32_t j = 0; j < state->selection.count; j++) {
				if (state->selection.selected_ids[j] == data->note.id) {
					for (uint32_t k = j; k < state->selection.count - 1; k++) {
						state->selection.selected_ids[k] =
							state->selection.selected_ids[k + 1];
					}
					state->selection.count--;
					break;
				}
			}

			return true;
		}
	}
	return false;
}

static bool redo_add_note(struct app_state *state, struct add_note_data *data)
{
	if (state->note_count >= UI_MAX_NOTES)
		return false;

	stored_to_ui_note(&data->note, &state->notes[state->note_count]);
	state->note_count++;
	return true;
}

static bool undo_delete_note(struct app_state *state, struct delete_note_data *data)
{
	if (state->note_count >= UI_MAX_NOTES)
		return false;

	if (data->index > state->note_count)
		data->index = state->note_count;

	for (uint32_t i = state->note_count; i > data->index; i--) {
		state->notes[i] = state->notes[i - 1];
	}

	stored_to_ui_note(&data->note, &state->notes[data->index]);
	state->note_count++;
	return true;
}

static bool redo_delete_note(struct app_state *state, struct delete_note_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note.id) {
			for (uint32_t j = i; j < state->note_count - 1; j++) {
				state->notes[j] = state->notes[j + 1];
			}
			state->note_count--;

			for (uint32_t j = 0; j < state->selection.count; j++) {
				if (state->selection.selected_ids[j] == data->note.id) {
					for (uint32_t k = j; k < state->selection.count - 1; k++) {
						state->selection.selected_ids[k] =
							state->selection.selected_ids[k + 1];
					}
					state->selection.count--;
					break;
				}
			}

			return true;
		}
	}
	return false;
}

static bool undo_move_note(struct app_state *state, struct move_note_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			int32_t new_ms = (int32_t)state->notes[i].ms - data->delta_ms;
			if (new_ms < 0)
				new_ms = 0;
			state->notes[i].ms = (uint32_t)new_ms;

			int32_t new_key =
				(int32_t)state->notes[i].piano_key - data->delta_piano_key;
			if (new_key < 0)
				new_key = 0;
			if (new_key > 127)
				new_key = 127;
			state->notes[i].piano_key = (uint8_t)new_key;
			return true;
		}
	}
	return false;
}

static bool redo_move_note(struct app_state *state, struct move_note_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			int32_t new_ms = (int32_t)state->notes[i].ms + data->delta_ms;
			if (new_ms < 0)
				new_ms = 0;
			state->notes[i].ms = (uint32_t)new_ms;

			int32_t new_key =
				(int32_t)state->notes[i].piano_key + data->delta_piano_key;
			if (new_key < 0)
				new_key = 0;
			if (new_key > 127)
				new_key = 127;
			state->notes[i].piano_key = (uint8_t)new_key;
			return true;
		}
	}
	return false;
}

static bool undo_resize_note(struct app_state *state, struct resize_note_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			if (data->from_left) {
				int32_t new_ms = (int32_t)state->notes[i].ms + data->delta_ms;
				int32_t new_duration = (int32_t)state->notes[i].duration_ms -
						       data->delta_duration_ms;

				if (new_ms < 0)
					new_ms = 0;
				if (new_duration < 10)
					new_duration = 10;

				state->notes[i].ms = (uint32_t)new_ms;
				state->notes[i].duration_ms = (uint16_t)new_duration;
			} else {
				int32_t new_duration = (int32_t)state->notes[i].duration_ms -
						       data->delta_duration_ms;
				if (new_duration < 10)
					new_duration = 10;
				state->notes[i].duration_ms = (uint16_t)new_duration;
			}
			return true;
		}
	}
	return false;
}

static bool redo_resize_note(struct app_state *state, struct resize_note_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			if (data->from_left) {
				int32_t new_ms = (int32_t)state->notes[i].ms - data->delta_ms;
				int32_t new_duration = (int32_t)state->notes[i].duration_ms +
						       data->delta_duration_ms;

				if (new_ms < 0)
					new_ms = 0;
				if (new_duration < 10)
					new_duration = 10;

				state->notes[i].ms = (uint32_t)new_ms;
				state->notes[i].duration_ms = (uint16_t)new_duration;
			} else {
				int32_t new_duration = (int32_t)state->notes[i].duration_ms +
						       data->delta_duration_ms;
				if (new_duration < 10)
					new_duration = 10;
				state->notes[i].duration_ms = (uint16_t)new_duration;
			}
			return true;
		}
	}
	return false;
}

static bool undo_set_note_voice(struct app_state *state, struct set_note_voice_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			state->notes[i].voice = data->old_voice;
			return true;
		}
	}
	return false;
}

static bool redo_set_note_voice(struct app_state *state, struct set_note_voice_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			state->notes[i].voice = data->new_voice;
			return true;
		}
	}
	return false;
}

static bool undo_set_note_instrument(struct app_state *state, struct set_note_instrument_data *data)
{
	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			state->notes[i].waveform = data->old_waveform;
			state->notes[i].duty_cycle = data->old_duty_cycle;
			state->notes[i].decay = data->old_decay;
			state->notes[i].amplitude_dbfs = data->old_amplitude_dbfs;
			state->notes[i].nes_noise_mode_flag = data->old_nes_noise_mode_flag;
			state->notes[i].nes_noise_lfsr_init = data->old_nes_noise_lfsr_init;
			return true;
		}
	}
	return false;
}

static bool redo_set_note_instrument(struct app_state *state, struct set_note_instrument_data *data)
{
	if (data->new_instrument_index >= state->instrument_count)
		return false;

	struct instrument *instr = &state->instruments[data->new_instrument_index];

	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == data->note_id) {
			state->notes[i].waveform = instr->waveform;
			state->notes[i].duty_cycle = instr->duty_cycle;
			state->notes[i].decay = instr->decay;
			state->notes[i].amplitude_dbfs = instr->amplitude_dbfs;
			state->notes[i].nes_noise_mode_flag = instr->nes_noise_mode_flag;
			state->notes[i].nes_noise_lfsr_init = instr->nes_noise_lfsr;
			return true;
		}
	}
	return false;
}

bool command_history_undo(struct command_history *history, struct app_state *state, void *audio)
{
	if (history == NULL || state == NULL || history->undo_count == 0)
		return false;

	uint32_t end_index = history->undo_count;
	uint32_t start_index = history->undo_count - 1;

	if (history->undo_stack[start_index].type == CMD_BATCH_END) {
		for (int32_t i = start_index - 1; i >= 0; i--) {
			if (history->undo_stack[i].type == CMD_BATCH_START) {
				start_index = i;
				break;
			}
		}
	}

	bool success = true;
	for (int32_t i = end_index - 1; i >= (int32_t)start_index; i--) {
		struct command *cmd = &history->undo_stack[i];

		if (cmd->type == CMD_BATCH_START || cmd->type == CMD_BATCH_END)
			continue;

		bool cmd_success = false;
		switch (cmd->type) {
		case CMD_ADD_NOTE:
			cmd_success = undo_add_note(state, &cmd->data.add_note);
			break;
		case CMD_DELETE_NOTE:
			cmd_success = undo_delete_note(state, &cmd->data.delete_note);
			break;
		case CMD_MOVE_NOTE:
			cmd_success = undo_move_note(state, &cmd->data.move_note);
			break;
		case CMD_RESIZE_NOTE:
			cmd_success = undo_resize_note(state, &cmd->data.resize_note);
			break;
		case CMD_SET_NOTE_VOICE:
			cmd_success = undo_set_note_voice(state, &cmd->data.set_note_voice);
			break;
		case CMD_SET_NOTE_INSTRUMENT:
			cmd_success =
				undo_set_note_instrument(state, &cmd->data.set_note_instrument);
			break;
		default:
			break;
		}

		if (!cmd_success)
			success = false;
	}

	uint32_t num_commands = end_index - start_index;
	for (uint32_t i = 0; i < num_commands; i++) {
		if (history->redo_count >= MAX_UNDO_HISTORY)
			break;
		history->redo_stack[history->redo_count] = history->undo_stack[start_index + i];
		history->redo_count++;
	}

	history->undo_count = start_index;

	if (audio != NULL) {
		struct sequencer *sequencer = audio_get_sequencer(audio);
		if (sequencer != NULL) {
			app_state_sync_notes_to_sequencer(state, sequencer, audio);
		}
	}

	return success;
}

bool command_history_redo(struct command_history *history, struct app_state *state, void *audio)
{
	if (history == NULL || state == NULL || history->redo_count == 0)
		return false;

	uint32_t start_index = history->redo_count - 1;
	uint32_t end_index = start_index + 1;

	if (history->redo_stack[start_index].type == CMD_BATCH_END) {
		for (int32_t i = start_index - 1; i >= 0; i--) {
			if (history->redo_stack[i].type == CMD_BATCH_START) {
				start_index = i;
				break;
			}
		}
	}

	bool success = true;
	for (uint32_t i = start_index; i < end_index; i++) {
		struct command *cmd = &history->redo_stack[i];

		if (cmd->type == CMD_BATCH_START || cmd->type == CMD_BATCH_END)
			continue;

		bool cmd_success = false;
		switch (cmd->type) {
		case CMD_ADD_NOTE:
			cmd_success = redo_add_note(state, &cmd->data.add_note);
			break;
		case CMD_DELETE_NOTE:
			cmd_success = redo_delete_note(state, &cmd->data.delete_note);
			break;
		case CMD_MOVE_NOTE:
			cmd_success = redo_move_note(state, &cmd->data.move_note);
			break;
		case CMD_RESIZE_NOTE:
			cmd_success = redo_resize_note(state, &cmd->data.resize_note);
			break;
		case CMD_SET_NOTE_VOICE:
			cmd_success = redo_set_note_voice(state, &cmd->data.set_note_voice);
			break;
		case CMD_SET_NOTE_INSTRUMENT:
			cmd_success =
				redo_set_note_instrument(state, &cmd->data.set_note_instrument);
			break;
		default:
			break;
		}

		if (!cmd_success)
			success = false;
	}

	uint32_t num_commands = end_index - start_index;
	for (uint32_t i = 0; i < num_commands; i++) {
		if (history->undo_count >= MAX_UNDO_HISTORY)
			break;
		history->undo_stack[history->undo_count] = history->redo_stack[start_index + i];
		history->undo_count++;
	}

	history->redo_count = start_index;

	if (audio != NULL) {
		struct sequencer *sequencer = audio_get_sequencer(audio);
		if (sequencer != NULL) {
			app_state_sync_notes_to_sequencer(state, sequencer, audio);
		}
	}

	return success;
}

void command_history_clear(struct command_history *history)
{
	if (history == NULL)
		return;

	history->undo_count = 0;
	history->redo_count = 0;
	history->in_batch = false;
	history->batch_start_index = 0;
}

void command_history_begin_batch(struct command_history *history)
{
	if (history == NULL || history->in_batch)
		return;

	struct command batch_cmd;
	batch_cmd.type = CMD_BATCH_START;
	command_history_push(history, batch_cmd);

	history->in_batch = true;
	history->batch_start_index = history->undo_count - 1;
}

void command_history_end_batch(struct command_history *history)
{
	if (history == NULL || !history->in_batch)
		return;

	struct command batch_cmd;
	batch_cmd.type = CMD_BATCH_END;
	command_history_push(history, batch_cmd);

	history->in_batch = false;
}
