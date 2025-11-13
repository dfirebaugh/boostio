#include "synth.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const uint16_t NES_APU_NOISE_PERIODS[16] = {
	4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

void synth_init(struct Synth *synth, uint32_t sample_rate)
{
	memset(synth, 0, sizeof(struct Synth));
	synth->sample_rate = sample_rate;
	synth->master_volume = 0.3f;
}

void synth_play_note_on_voice(struct Synth *synth, struct NoteParams params, uint8_t voice_index)
{
	if (voice_index >= MAX_VOICES) {
		return;
	}

	struct Voice *voice = &synth->voices[voice_index];
	voice->frequency = params.frequency;
	voice->amplitude = dbfs_to_amplitude(params.amplitude_dbfs);

	if (params.restart_phase) {
		voice->phase = 0.0f;
	}

	voice->waveform = params.waveform;
	voice->active = true;
	voice->duration_ms = params.duration_ms;
	voice->elapsed_ms = 0.0f;
	voice->duty_cycle = params.duty_cycle;
	voice->decay = params.decay;
	voice->amplitude_dbfs = params.amplitude_dbfs;
	voice->nes_noise_period = params.nes_noise_period;
	voice->nes_noise_lfsr = params.nes_noise_lfsr_init;
	voice->nes_noise_mode_flag = params.nes_noise_mode_flag;
}

void synth_play_note(struct Synth *synth, struct NoteParams params)
{
	if (params.voice_index >= 0 && params.voice_index < MAX_VOICES) {
		synth_play_note_on_voice(synth, params, (uint8_t)params.voice_index);
		return;
	}

	for (int i = 0; i < MAX_VOICES; i++) {
		if (!synth->voices[i].active) {
			synth_play_note_on_voice(synth, params, i);
			return;
		}
	}
}

void synth_stop_all(struct Synth *synth)
{
	for (int i = 0; i < MAX_VOICES; i++) {
		synth->voices[i].active = false;
	}
}

float dbfs_to_amplitude(int8_t dbfs)
{
	return powf(10.0f, (float)dbfs / 20.0f);
}

static float generate_waveform_with_duty(enum WaveformType type, float phase, uint8_t duty_cycle)
{
	switch (type) {
	case WAVEFORM_SINE:
		return sinf(phase * 2.0f * M_PI);

	case WAVEFORM_SQUARE: {
		float duty = (float)duty_cycle / 255.0f;
		return (phase < duty) ? 1.0f : -1.0f;
	}

	case WAVEFORM_TRIANGLE:
		if (phase < 0.5f) {
			return 4.0f * phase - 1.0f;
		} else {
			return 3.0f - 4.0f * phase;
		}

	case WAVEFORM_SAWTOOTH:
		return 2.0f * phase - 1.0f;

	default:
		return 0.0f;
	}
}

void synth_generate_samples(struct Synth *synth, float *buffer, uint32_t num_samples)
{
	memset(buffer, 0, num_samples * sizeof(float));

	float ms_per_sample = 1000.0f / (float)synth->sample_rate;

	for (int v = 0; v < MAX_VOICES; v++) {
		if (!synth->voices[v].active) {
			continue;
		}

		struct Voice *voice = &synth->voices[v];

		for (uint32_t i = 0; i < num_samples; i++) {
			if (voice->elapsed_ms >= voice->duration_ms) {
				voice->active = false;
				break;
			}

			float samples_elapsed =
				voice->elapsed_ms * (float)synth->sample_rate / 1000.0f;

			float final_amplitude = voice->amplitude;
			if (voice->decay != 0) {
				float decay_per_sample =
					(voice->amplitude * (float)abs(voice->decay)) / 4800000.0f;
				float total_decay = decay_per_sample * samples_elapsed;

				if (voice->decay > 0) {
					final_amplitude =
						fmaxf(0.0f, voice->amplitude - total_decay);
				} else {
					final_amplitude =
						fminf(1.0f, voice->amplitude + total_decay);
				}
			}

			float envelope = 1.0f;
			float attack_ms = 10.0f;
			float release_ms = 50.0f;

			if (voice->elapsed_ms < attack_ms) {
				envelope = voice->elapsed_ms / attack_ms;
			} else if (voice->elapsed_ms > voice->duration_ms - release_ms) {
				envelope = (voice->duration_ms - voice->elapsed_ms) / release_ms;
			}

			float sample = 0.0f;

			if (voice->waveform == WAVEFORM_NES_NOISE) {
				uint8_t period_index = (voice->nes_noise_period > 15)
							       ? 15
							       : voice->nes_noise_period;
				uint16_t apu_period = NES_APU_NOISE_PERIODS[period_index];

				float apu_rate = 894886.0f;
				uint32_t period = (uint32_t)fmaxf(
					1.0f,
					((float)apu_period * (float)synth->sample_rate) / apu_rate
				);

				if (((uint32_t)voice->phase) % period == 0) {
					uint16_t bit0 = voice->nes_noise_lfsr & 0x0001;
					uint16_t bit_tap;
					if (voice->nes_noise_mode_flag) {
						bit_tap = (voice->nes_noise_lfsr >> 6) & 0x0001;
					} else {
						bit_tap = (voice->nes_noise_lfsr >> 1) & 0x0001;
					}

					uint16_t feedback = (bit0 ^ bit_tap) << 14;
					voice->nes_noise_lfsr >>= 1;
					voice->nes_noise_lfsr |= feedback;
				}

				sample = (voice->nes_noise_lfsr & 0x0001) ? 1.0f : -1.0f;
			} else {
				sample = generate_waveform_with_duty(
					voice->waveform, voice->phase, voice->duty_cycle
				);
			}

			sample *= final_amplitude * envelope;
			buffer[i] += sample;

			if (voice->waveform == WAVEFORM_NES_NOISE) {
				voice->phase += 1.0f;
			} else {
				voice->phase += voice->frequency / (float)synth->sample_rate;
				if (voice->phase >= 1.0f) {
					voice->phase -= 1.0f;
				}
			}

			voice->elapsed_ms += ms_per_sample;
		}
	}

	for (uint32_t i = 0; i < num_samples; i++) {
		buffer[i] *= synth->master_volume;

		if (buffer[i] > 1.0f) {
			buffer[i] = 1.0f;
		} else if (buffer[i] < -1.0f) {
			buffer[i] = -1.0f;
		}
	}
}

float note_to_frequency(int note)
{
	return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}
