#ifndef SYNTH_H
#define SYNTH_H

#include <stdbool.h>
#include <stdint.h>

enum WaveformType
{
	WAVEFORM_SINE,
	WAVEFORM_SQUARE,
	WAVEFORM_TRIANGLE,
	WAVEFORM_SAWTOOTH,
	WAVEFORM_NES_NOISE
};

struct Voice
{
	float frequency;
	float amplitude;
	float phase;
	enum WaveformType waveform;
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

struct Synth
{
	struct Voice voices[MAX_VOICES];
	uint32_t sample_rate;
	float master_volume;
};

struct NoteParams
{
	float frequency;
	float duration_ms;
	enum WaveformType waveform;
	uint8_t duty_cycle;
	int16_t decay;
	int8_t amplitude_dbfs;
	uint8_t nes_noise_period;
	bool nes_noise_mode_flag;
	int8_t voice_index;
	bool restart_phase;
	uint16_t nes_noise_lfsr_init;
};

void synth_init(struct Synth *synth, uint32_t sample_rate);
void synth_play_note(struct Synth *synth, struct NoteParams params);
void synth_play_note_on_voice(struct Synth *synth, struct NoteParams params, uint8_t voice_index);
void synth_stop_all(struct Synth *synth);
void synth_generate_samples(struct Synth *synth, float *buffer, uint32_t num_samples);

float note_to_frequency(int note);
float dbfs_to_amplitude(int8_t dbfs);

#endif
