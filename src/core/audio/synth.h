#ifndef SYNTH_H
#define SYNTH_H

#include <stdbool.h>
#include <stdint.h>

enum waveform_type {
	WAVEFORM_SINE,
	WAVEFORM_SQUARE,
	WAVEFORM_TRIANGLE,
	WAVEFORM_SAWTOOTH,
	WAVEFORM_NES_NOISE
};

struct voice {
	float frequency;
	float amplitude;
	float phase;
	enum waveform_type waveform;
	bool active;
	float duration_ms;
	float elapsed_ms;

	uint8_t duty_cycle;
	int16_t decay;
	int8_t amplitude_dbfs;

	uint8_t nes_noise_period;
	uint16_t nes_noise_lfsr;
	bool nes_noise_mode_flag;
};

#define MAX_VOICES 16

struct synth {
	struct voice voices[MAX_VOICES];
	uint32_t sample_rate;
	float master_volume;
};

struct note_params {
	float frequency;
	float duration_ms;
	enum waveform_type waveform;
	uint8_t duty_cycle;
	int16_t decay;
	int8_t amplitude_dbfs;
	uint8_t nes_noise_period;
	bool nes_noise_mode_flag;
	int8_t voice_index;
	bool restart_phase;
	uint16_t nes_noise_lfsr_init;
	uint8_t piano_key;
};

void synth_init(struct synth *synth, uint32_t sample_rate);
void synth_play_note(struct synth *synth, struct note_params params);
void synth_generate_samples(struct synth *synth, float *buffer, uint32_t num_samples);

float note_to_frequency(int note);
float dbfs_to_amplitude(int8_t dbfs);

#endif
