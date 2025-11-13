#include "audio.h"
#include "sequencer.h"
#include "synth.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

struct Audio {
	SDL_AudioDeviceID device_id;
	SDL_AudioStream *stream;
	struct Synth synth;
	struct Sequencer sequencer;
	bool initialized;
	uint32_t sample_rate;
	uint32_t last_update_time;
};

struct Audio *audio_create(void)
{
	struct Audio *audio = malloc(sizeof(struct Audio));
	if (!audio) {
		return NULL;
	}

	memset(audio, 0, sizeof(struct Audio));

	audio->sample_rate = 48000;

	SDL_AudioSpec spec;
	spec.freq = audio->sample_rate;
	spec.format = SDL_AUDIO_F32;
	spec.channels = 1;

	audio->device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	if (audio->device_id == 0) {
		SDL_Log("Failed to open audio device: %s", SDL_GetError());
		free(audio);
		return NULL;
	}

	audio->stream = SDL_CreateAudioStream(&spec, &spec);
	if (!audio->stream) {
		SDL_Log("Failed to create audio stream: %s", SDL_GetError());
		SDL_CloseAudioDevice(audio->device_id);
		free(audio);
		return NULL;
	}

	if (!SDL_BindAudioStream(audio->device_id, audio->stream)) {
		SDL_Log("Failed to bind audio stream: %s", SDL_GetError());
		SDL_DestroyAudioStream(audio->stream);
		SDL_CloseAudioDevice(audio->device_id);
		free(audio);
		return NULL;
	}

	synth_init(&audio->synth, audio->sample_rate);
	sequencer_init(&audio->sequencer);

	audio->initialized = true;
	audio->last_update_time = SDL_GetTicks();

	SDL_Log("Audio system initialized: %d Hz", audio->sample_rate);

	return audio;
}

void audio_destroy(struct Audio *audio)
{
	if (!audio) {
		return;
	}

	if (audio->stream) {
		SDL_DestroyAudioStream(audio->stream);
	}

	if (audio->device_id) {
		SDL_CloseAudioDevice(audio->device_id);
	}

	free(audio);
}

bool audio_is_initialized(const struct Audio *audio)
{
	return audio && audio->initialized;
}

void audio_start(struct Audio *audio)
{
	if (!audio || !audio->initialized) {
		return;
	}

	SDL_ResumeAudioDevice(audio->device_id);
	SDL_Log("Audio device resumed");
}

void audio_stop(struct Audio *audio)
{
	if (!audio || !audio->initialized) {
		return;
	}

	SDL_PauseAudioDevice(audio->device_id);
}

void audio_update(struct Audio *audio, const bool *voice_solo, const bool *voice_muted)
{
	if (!audio || !audio->initialized) {
		return;
	}

	uint32_t current_time = SDL_GetTicks();
	float delta_time_ms = (float)(current_time - audio->last_update_time);
	audio->last_update_time = current_time;

	sequencer_update(&audio->sequencer, &audio->synth, delta_time_ms, voice_solo, voice_muted);

	const uint32_t buffer_size = 2048;
	const uint32_t min_queued_bytes = buffer_size * sizeof(float) * 2;

	int queued = SDL_GetAudioStreamQueued(audio->stream);

	while (queued >= 0 && (uint32_t)queued < min_queued_bytes) {
		float buffer[buffer_size];
		synth_generate_samples(&audio->synth, buffer, buffer_size);
		SDL_PutAudioStreamData(audio->stream, buffer, buffer_size * sizeof(float));
		queued = SDL_GetAudioStreamQueued(audio->stream);
	}
}

struct Synth *audio_get_synth(struct Audio *audio)
{
	if (!audio) {
		return NULL;
	}
	return &audio->synth;
}

struct Sequencer *audio_get_sequencer(struct Audio *audio)
{
	if (!audio) {
		return NULL;
	}
	return &audio->sequencer;
}
