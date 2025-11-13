#include "wav_exporter.h"
#include "sequencer.h"
#include "synth.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 48000
#define BITS_PER_SAMPLE 16
#define CHANNELS 1
#define BUFFER_SIZE 1024
#define WAV_MAX_VOICES 8

static const uint16_t NES_APU_NOISE_PERIODS[16] = {
	4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

struct render_voice {
	bool active;
	uint32_t note_index;
	float phase;
	float amplitude;
	uint32_t start_time_ms;
	uint16_t lfsr;
	struct note_params params;
	uint32_t start_ms;
};

static void trigger_note(struct render_voice *voice, const struct note *note)
{
	voice->active = true;
	voice->params = note->params;
	voice->start_ms = note->time_ms;
	voice->phase = 0.0f;

	float db = (float)note->params.amplitude_dbfs;
	voice->amplitude = powf(10.0f, db / 20.0f);

	if (note->params.waveform == WAVEFORM_NES_NOISE) {
		voice->lfsr = note->params.nes_noise_lfsr_init;
	}
}

static float generate_square(float phase, float frequency, uint8_t duty_cycle)
{
	float period = (float)SAMPLE_RATE / frequency;
	float duty = (float)duty_cycle / 255.0f;
	float phase_in_period = fmodf(phase, period);
	return (phase_in_period < period * duty) ? 1.0f : -1.0f;
}

static float generate_triangle(float phase, float frequency)
{
	float period = (float)SAMPLE_RATE / frequency;
	float phase_in_period = fmodf(phase, period);
	float normalized = phase_in_period / period;
	return (normalized < 0.5f) ? (4.0f * normalized - 1.0f) : (3.0f - 4.0f * normalized);
}

static float generate_sawtooth(float phase, float frequency)
{
	float period = (float)SAMPLE_RATE / frequency;
	float phase_in_period = fmodf(phase, period);
	return 2.0f * (phase_in_period / period) - 1.0f;
}

static float generate_sine(float phase, float frequency)
{
	float phase_radians = phase * 2.0f * M_PI * frequency / (float)SAMPLE_RATE;
	return sinf(phase_radians);
}

static float generate_nes_noise(struct render_voice *voice, uint8_t period_index, bool mode_flag)
{
	uint8_t idx = (period_index > 15) ? 15 : period_index;
	uint16_t apu_period = NES_APU_NOISE_PERIODS[idx];

	float apu_rate = 894886.0f;
	uint32_t period = (uint32_t)((float)apu_period * (float)SAMPLE_RATE / apu_rate);
	if (period < 1)
		period = 1;

	if (((uint32_t)voice->phase % period) == 0) {
		uint16_t bit0 = voice->lfsr & 0x0001;
		uint16_t bit_tap;

		if (mode_flag) {
			bit_tap = (voice->lfsr >> 6) & 0x0001;
		} else {
			bit_tap = (voice->lfsr >> 1) & 0x0001;
		}

		uint16_t feedback = (bit0 ^ bit_tap) << 14;
		voice->lfsr >>= 1;
		voice->lfsr |= feedback;
	}

	return ((voice->lfsr & 0x0001) != 0) ? 1.0f : -1.0f;
}

static float generate_sample(struct render_voice *voice, uint32_t current_time_ms)
{
	if (!voice->active)
		return 0.0f;

	float sample = 0.0f;

	switch (voice->params.waveform) {
	case WAVEFORM_SQUARE:
		sample = generate_square(
			voice->phase, voice->params.frequency, voice->params.duty_cycle
		);
		break;
	case WAVEFORM_TRIANGLE:
		sample = generate_triangle(voice->phase, voice->params.frequency);
		break;
	case WAVEFORM_SAWTOOTH:
		sample = generate_sawtooth(voice->phase, voice->params.frequency);
		break;
	case WAVEFORM_SINE:
		sample = generate_sine(voice->phase, voice->params.frequency);
		break;
	case WAVEFORM_NES_NOISE:
		sample = generate_nes_noise(
			voice, voice->params.nes_noise_period, voice->params.nes_noise_mode_flag
		);
		break;
	}

	float samples_elapsed = voice->phase;
	float final_amplitude = voice->amplitude;

	if (voice->params.decay != 0) {
		float decay_per_sample =
			(voice->amplitude * (float)abs(voice->params.decay)) / 4800000.0f;
		float total_decay = decay_per_sample * samples_elapsed;

		if (voice->params.decay > 0) {
			final_amplitude = fmaxf(0.0f, voice->amplitude - total_decay);
		} else {
			final_amplitude = fminf(1.0f, voice->amplitude + total_decay);
		}
	}

	voice->phase += 1.0f;

	return sample * final_amplitude * 0.2f;
}

static void write_wav_header(FILE *file, uint32_t data_size)
{
	fwrite("RIFF", 1, 4, file);

	uint32_t file_size = 36 + data_size;
	fwrite(&file_size, 4, 1, file);

	fwrite("WAVE", 1, 4, file);

	fwrite("fmt ", 1, 4, file);

	uint32_t fmt_size = 16;
	fwrite(&fmt_size, 4, 1, file);

	uint16_t audio_format = 1;
	fwrite(&audio_format, 2, 1, file);

	uint16_t num_channels = CHANNELS;
	fwrite(&num_channels, 2, 1, file);

	uint32_t sample_rate = SAMPLE_RATE;
	fwrite(&sample_rate, 4, 1, file);

	uint32_t byte_rate = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8);
	fwrite(&byte_rate, 4, 1, file);

	uint16_t block_align = CHANNELS * (BITS_PER_SAMPLE / 8);
	fwrite(&block_align, 2, 1, file);

	uint16_t bits_per_sample = BITS_PER_SAMPLE;
	fwrite(&bits_per_sample, 2, 1, file);

	fwrite("data", 1, 4, file);
	fwrite(&data_size, 4, 1, file);
}

static uint32_t calculate_duration_ms(const struct sequencer *sequencer)
{
	uint32_t max_end = 0;

	for (uint32_t i = 0; i < sequencer->note_count; i++) {
		const struct note *note = &sequencer->notes[i];
		uint32_t end_time = note->time_ms + (uint32_t)note->params.duration_ms;
		if (end_time > max_end) {
			max_end = end_time;
		}
	}

	return (max_end > 0) ? max_end : 1000;
}

bool wav_exporter_export_to_file(const struct sequencer *sequencer, const char *filepath)
{
	if (!sequencer || !filepath) {
		fprintf(stderr, "Invalid parameters for WAV export\n");
		return false;
	}

	uint32_t duration_ms = calculate_duration_ms(sequencer);
	uint64_t total_samples = ((uint64_t)duration_ms * SAMPLE_RATE) / 1000;
	uint32_t data_size = (uint32_t)(total_samples * CHANNELS * (BITS_PER_SAMPLE / 8));

	FILE *file = fopen(filepath, "wb");
	if (!file) {
		fprintf(stderr, "Failed to open file for WAV export: %s\n", filepath);
		return false;
	}

	write_wav_header(file, data_size);

	struct render_voice voices[WAV_MAX_VOICES];
	memset(voices, 0, sizeof(voices));

	bool note_triggered[SEQUENCER_MAX_NOTES];
	memset(note_triggered, 0, sizeof(note_triggered));

	uint64_t current_sample = 0;

	int16_t sample_buffer[BUFFER_SIZE];
	size_t buffer_index = 0;

	while (current_sample < total_samples) {
		uint32_t current_time_ms = (uint32_t)((current_sample * 1000) / SAMPLE_RATE);

		for (uint32_t i = 0; i < sequencer->note_count; i++) {
			if (!note_triggered[i]) {
				const struct note *note = &sequencer->notes[i];
				if (note->time_ms <= current_time_ms) {
					int8_t voice_idx = note->params.voice_index;
					if (voice_idx >= 0 && voice_idx < WAV_MAX_VOICES) {
						trigger_note(&voices[voice_idx], note);
					}
					note_triggered[i] = true;
				}
			}
		}

		for (int i = 0; i < WAV_MAX_VOICES; i++) {
			struct render_voice *voice = &voices[i];
			if (voice->active) {
				uint32_t note_end =
					voice->start_ms + (uint32_t)voice->params.duration_ms;
				if (current_time_ms >= note_end) {
					voice->active = false;
				}
			}
		}

		float mixed_sample = 0.0f;
		for (int i = 0; i < WAV_MAX_VOICES; i++) {
			if (voices[i].active) {
				mixed_sample += generate_sample(&voices[i], current_time_ms);
			}
		}

		float clamped = fmaxf(-1.0f, fminf(1.0f, mixed_sample));
		int16_t sample_i16 = (int16_t)(clamped * 32767.0f);

		sample_buffer[buffer_index] = sample_i16;
		buffer_index++;

		if (buffer_index >= BUFFER_SIZE) {
			fwrite(sample_buffer, sizeof(int16_t), buffer_index, file);
			buffer_index = 0;
		}

		current_sample++;
	}

	if (buffer_index > 0) {
		fwrite(sample_buffer, sizeof(int16_t), buffer_index, file);
	}

	fclose(file);

	printf("Exported %llu samples (%u ms) to WAV file: %s\n",
	       (unsigned long long)total_samples,
	       duration_ms,
	       filepath);
	return true;
}
