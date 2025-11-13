#include "song_loader.h"
#include "audio.h"
#include "cJSON.h"
#include "sequencer.h"
#include "synth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static enum WaveformType string_to_waveform(const char *str)
{
	if (strcmp(str, "sine") == 0)
		return WAVEFORM_SINE;
	if (strcmp(str, "square") == 0)
		return WAVEFORM_SQUARE;
	if (strcmp(str, "triangle") == 0)
		return WAVEFORM_TRIANGLE;
	if (strcmp(str, "sawtooth") == 0)
		return WAVEFORM_SAWTOOTH;
	if (strcmp(str, "nes_noise") == 0)
		return WAVEFORM_NES_NOISE;

	return WAVEFORM_SINE;
}

static uint8_t frequency_to_piano_key(float frequency)
{
	float midi_note = 69.0f + 12.0f * log2f(frequency / 440.0f);
	int32_t rounded = (int32_t)(midi_note + 0.5f);

	if (rounded < 0)
		return 0;
	if (rounded > 127)
		return 127;

	return (uint8_t)rounded;
}

static char *read_file(const char *filepath)
{
	FILE *file = fopen(filepath, "rb");
	if (!file)
	{
		fprintf(stderr, "Failed to open file: %s\n", filepath);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *content = malloc(length + 1);
	if (!content)
	{
		fclose(file);
		return NULL;
	}

	size_t read_length = fread(content, 1, length, file);
	content[read_length] = '\0';
	fclose(file);

	return content;
}

bool song_loader_load_from_file(struct Audio *audio, const char *filepath)
{
	char *json_content = read_file(filepath);
	if (!json_content)
	{
		return false;
	}

	cJSON *root = cJSON_Parse(json_content);
	free(json_content);

	if (!root)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr)
		{
			fprintf(stderr, "JSON parse error before: %s\n", error_ptr);
		}
		return false;
	}

	struct Sequencer *sequencer = audio_get_sequencer(audio);

	sequencer_clear_notes(sequencer);
	sequencer_stop(sequencer);

	cJSON *bpm_item = cJSON_GetObjectItemCaseSensitive(root, "bpm");
	if (cJSON_IsNumber(bpm_item))
	{
		sequencer_set_bpm(sequencer, (uint32_t)bpm_item->valueint);
		printf("Loaded song with BPM: %d\n", bpm_item->valueint);
	}

	cJSON *notes_array = cJSON_GetObjectItemCaseSensitive(root, "notes");
	if (!cJSON_IsArray(notes_array))
	{
		fprintf(stderr, "No notes array found in JSON\n");
		cJSON_Delete(root);
		return false;
	}

	int note_count = 0;
	cJSON *note_item = NULL;
	cJSON_ArrayForEach(note_item, notes_array)
	{
		struct NoteParams params = {
				.frequency = 440.0f,
				.duration_ms = 200.0f,
				.waveform = WAVEFORM_SINE,
				.duty_cycle = 128,
				.decay = 0,
				.amplitude_dbfs = -3,
				.nes_noise_period = 15,
				.nes_noise_mode_flag = false,
				.voice_index = -1,
				.restart_phase = true,
				.nes_noise_lfsr_init = 0xFFFF,
				.piano_key = 60
		};

		cJSON *ms = cJSON_GetObjectItemCaseSensitive(note_item, "ms");
		cJSON *voice = cJSON_GetObjectItemCaseSensitive(note_item, "voice");
		cJSON *frequency_hz = cJSON_GetObjectItemCaseSensitive(note_item, "frequency_hz");
		cJSON *duration_ms = cJSON_GetObjectItemCaseSensitive(note_item, "duration_ms");
		cJSON *waveform = cJSON_GetObjectItemCaseSensitive(note_item, "waveform");
		cJSON *amplitude_dbfs = cJSON_GetObjectItemCaseSensitive(note_item, "amplitude_dbfs");
		cJSON *duty_cycle = cJSON_GetObjectItemCaseSensitive(note_item, "duty_cycle");
		cJSON *decay = cJSON_GetObjectItemCaseSensitive(note_item, "decay");
		cJSON *nes_noise_period = cJSON_GetObjectItemCaseSensitive(note_item, "nes_noise_period");
		cJSON *nes_noise_mode = cJSON_GetObjectItemCaseSensitive(note_item, "nes_noise_mode");
		cJSON *restart = cJSON_GetObjectItemCaseSensitive(note_item, "restart");
		cJSON *nes_noise_lfsr = cJSON_GetObjectItemCaseSensitive(note_item, "nes_noise_lfsr");
		cJSON *piano_key_json = cJSON_GetObjectItemCaseSensitive(note_item, "piano_key");

		if (!cJSON_IsNumber(ms))
		{
			fprintf(stderr, "Note missing 'ms' field, skipping\n");
			continue;
		}

		uint32_t start_ms = (uint32_t)ms->valueint;

		if (cJSON_IsNumber(voice))
		{
			params.voice_index = (int8_t)voice->valueint;
		}

		if (cJSON_IsNumber(frequency_hz))
		{
			params.frequency = (float)frequency_hz->valuedouble;
			params.piano_key = frequency_to_piano_key(params.frequency);
		}

		if (cJSON_IsNumber(duration_ms))
		{
			params.duration_ms = (float)duration_ms->valueint;
		}

		if (cJSON_IsString(waveform))
		{
			params.waveform = string_to_waveform(waveform->valuestring);
		}

		if (cJSON_IsNumber(amplitude_dbfs))
		{
			params.amplitude_dbfs = (int8_t)amplitude_dbfs->valueint;
		}

		if (cJSON_IsNumber(duty_cycle))
		{
			params.duty_cycle = (uint8_t)duty_cycle->valueint;
		}

		if (cJSON_IsNumber(decay))
		{
			params.decay = (int16_t)decay->valueint;
		}

		if (cJSON_IsNumber(nes_noise_period))
		{
			params.nes_noise_period = (uint8_t)nes_noise_period->valueint;
		}

		if (cJSON_IsBool(nes_noise_mode))
		{
			params.nes_noise_mode_flag = cJSON_IsTrue(nes_noise_mode);
		}

		if (cJSON_IsBool(restart))
		{
			params.restart_phase = cJSON_IsTrue(restart);
		}

		if (cJSON_IsNumber(nes_noise_lfsr))
		{
			params.nes_noise_lfsr_init = (uint16_t)nes_noise_lfsr->valueint;
		}

		if (cJSON_IsNumber(piano_key_json) && !cJSON_IsNumber(frequency_hz))
		{
			params.piano_key = (uint8_t)piano_key_json->valueint;
		}

		sequencer_add_note(sequencer, start_ms, params);
		note_count++;
	}

	cJSON_Delete(root);

	printf("Loaded %d notes from %s\n", note_count, filepath);
	return true;
}
