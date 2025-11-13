#include "audio.h"
#include "sequencer.h"
#include "synth.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

struct audio {
	SDL_AudioDeviceID device_id;
	SDL_AudioStream *stream;
	struct synth synth;
	struct sequencer sequencer;
	bool initialized;
	uint32_t sample_rate;
	uint32_t last_update_time;
	bool voice_solo_cache[8];
	bool voice_muted_cache[8];
};

static void audio_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
	struct audio *audio = (struct audio *)userdata;
	if (!audio) {
		return;
	}

	const uint32_t buffer_size = 2048;
	int samples_needed = additional_amount / sizeof(float);

	while (samples_needed > 0) {
		uint32_t samples = (samples_needed > (int)buffer_size) ? buffer_size : (uint32_t)samples_needed;
		float buffer[buffer_size];

		sequencer_update(&audio->sequencer, &audio->synth, samples, audio->voice_solo_cache, audio->voice_muted_cache);

		synth_generate_samples(&audio->synth, buffer, samples);
		SDL_PutAudioStreamData(stream, buffer, samples * sizeof(float));

		samples_needed -= samples;
	}
}

struct audio *audio_create(void)
{
	struct audio *audio = malloc(sizeof(struct audio));
	if (!audio) {
		return NULL;
	}

	memset(audio, 0, sizeof(struct audio));

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
	audio->sequencer.sample_rate = audio->sample_rate;

	memset(audio->voice_solo_cache, 0, sizeof(audio->voice_solo_cache));
	memset(audio->voice_muted_cache, 0, sizeof(audio->voice_muted_cache));

	if (!SDL_SetAudioStreamGetCallback(audio->stream, audio_callback, audio)) {
		SDL_Log("Failed to set audio stream callback: %s", SDL_GetError());
		SDL_DestroyAudioStream(audio->stream);
		SDL_CloseAudioDevice(audio->device_id);
		free(audio);
		return NULL;
	}

	audio->initialized = true;
	audio->last_update_time = SDL_GetTicks();

	SDL_Log("Audio system initialized: %d Hz", audio->sample_rate);

	return audio;
}

void audio_destroy(struct audio *audio)
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

bool audio_is_initialized(const struct audio *audio)
{
	return audio && audio->initialized;
}

void audio_start(struct audio *audio)
{
	if (!audio || !audio->initialized) {
		return;
	}

	SDL_ResumeAudioDevice(audio->device_id);
	SDL_Log("Audio device resumed");
}

void audio_stop(struct audio *audio)
{
	if (!audio || !audio->initialized) {
		return;
	}

	SDL_PauseAudioDevice(audio->device_id);
}

void audio_update(struct audio *audio, const bool *voice_solo, const bool *voice_muted)
{
	if (!audio || !audio->initialized) {
		return;
	}

	if (voice_solo && voice_muted) {
		SDL_LockAudioStream(audio->stream);
		memcpy(audio->voice_solo_cache, voice_solo, 8 * sizeof(bool));
		memcpy(audio->voice_muted_cache, voice_muted, 8 * sizeof(bool));
		SDL_UnlockAudioStream(audio->stream);
	}
}

struct synth *audio_get_synth(struct audio *audio)
{
	if (!audio) {
		return NULL;
	}
	return &audio->synth;
}

struct sequencer *audio_get_sequencer(struct audio *audio)
{
	if (!audio) {
		return NULL;
	}
	return &audio->sequencer;
}
