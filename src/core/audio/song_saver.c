#include "song_saver.h"
#include "app/app_state.h"
#include "cJSON.h"
#include "scale.h"
#include "sequencer.h"
#include "synth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *waveform_to_string(enum WaveformType waveform)
{
	switch (waveform) {
	case WAVEFORM_SINE:
		return "sine";
	case WAVEFORM_SQUARE:
		return "square";
	case WAVEFORM_TRIANGLE:
		return "triangle";
	case WAVEFORM_SAWTOOTH:
		return "sawtooth";
	case WAVEFORM_NES_NOISE:
		return "nes_noise";
	default:
		return "sine";
	}
}

static uint32_t calculate_song_length_ms(const struct Sequencer *sequencer)
{
	uint32_t max_end_time = 0;

	for (uint32_t i = 0; i < sequencer->note_count; i++) {
		const struct Note *note = &sequencer->notes[i];
		uint32_t end_time = note->time_ms + (uint32_t)note->params.duration_ms;

		if (end_time > max_end_time) {
			max_end_time = end_time;
		}
	}

	return max_end_time > 0 ? max_end_time : 1000;
}

bool song_saver_save_to_file(
	const struct app_state *state, const struct Sequencer *sequencer, const char *filepath
)
{
	if (!state || !sequencer || !filepath) {
		fprintf(stderr, "Invalid parameters for save\n");
		return false;
	}

	cJSON *root = cJSON_CreateObject();
	if (!root) {
		fprintf(stderr, "Failed to create JSON object\n");
		return false;
	}

	cJSON_AddStringToObject(root, "version", "1.0");
	cJSON_AddNumberToObject(root, "bpm", sequencer->bpm);

	uint32_t length_ms = calculate_song_length_ms(sequencer);
	cJSON_AddNumberToObject(root, "length_ms", length_ms);

	const char *scale_str = scale_type_to_string(state->selected_scale);
	cJSON_AddStringToObject(root, "selected_scale", scale_str);

	const char *root_str = root_note_to_string(state->selected_root);
	cJSON_AddStringToObject(root, "selected_root", root_str);

	cJSON_AddBoolToObject(root, "fold_mode", state->fold_mode);
	cJSON_AddBoolToObject(root, "show_scale_highlights", state->show_scale_highlights);

	cJSON *notes_array = cJSON_CreateArray();
	if (!notes_array) {
		fprintf(stderr, "Failed to create notes array\n");
		cJSON_Delete(root);
		return false;
	}

	for (uint32_t i = 0; i < sequencer->note_count; i++) {
		const struct Note *note = &sequencer->notes[i];
		const struct NoteParams *params = &note->params;

		cJSON *note_obj = cJSON_CreateObject();
		if (!note_obj) {
			fprintf(stderr, "Failed to create note object\n");
			continue;
		}

		cJSON_AddNumberToObject(note_obj, "id", i + 1);
		cJSON_AddNumberToObject(note_obj, "ms", note->time_ms);
		cJSON_AddNumberToObject(note_obj, "voice", params->voice_index);
		cJSON_AddNumberToObject(note_obj, "frequency_hz", params->frequency);
		cJSON_AddNumberToObject(note_obj, "duration_ms", (int)params->duration_ms);

		const char *waveform_str = waveform_to_string(params->waveform);
		cJSON_AddStringToObject(note_obj, "waveform", waveform_str);

		cJSON_AddNumberToObject(note_obj, "amplitude_dbfs", params->amplitude_dbfs);
		cJSON_AddNumberToObject(note_obj, "duty_cycle", params->duty_cycle);
		cJSON_AddNumberToObject(note_obj, "decay", params->decay);
		cJSON_AddNumberToObject(note_obj, "phase", 0);
		cJSON_AddBoolToObject(note_obj, "restart", params->restart_phase);
		cJSON_AddBoolToObject(note_obj, "nes_noise_mode", params->nes_noise_mode_flag);
		cJSON_AddNumberToObject(note_obj, "nes_noise_lfsr", params->nes_noise_lfsr_init);
		cJSON_AddNumberToObject(note_obj, "nes_noise_period", params->nes_noise_period);
		cJSON_AddNumberToObject(note_obj, "piano_key", params->piano_key);

		cJSON_AddItemToArray(notes_array, note_obj);
	}

	cJSON_AddItemToObject(root, "notes", notes_array);

	char *json_string = cJSON_Print(root);
	if (!json_string) {
		fprintf(stderr, "Failed to serialize JSON\n");
		cJSON_Delete(root);
		return false;
	}

	FILE *file = fopen(filepath, "w");
	if (!file) {
		fprintf(stderr, "Failed to open file for writing: %s\n", filepath);
		free(json_string);
		cJSON_Delete(root);
		return false;
	}

	size_t json_length = strlen(json_string);
	size_t written = fwrite(json_string, 1, json_length, file);

	fclose(file);
	free(json_string);
	cJSON_Delete(root);

	if (written != json_length) {
		fprintf(stderr, "Failed to write complete JSON to file\n");
		return false;
	}

	printf("Saved %u notes to %s\n", sequencer->note_count, filepath);
	return true;
}
