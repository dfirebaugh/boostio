#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

struct audio;
struct synth;
struct sequencer;

struct audio *audio_create(void);
void audio_destroy(struct audio *audio);

bool audio_is_initialized(const struct audio *audio);

void audio_start(struct audio *audio);
void audio_stop(struct audio *audio);

void audio_update(struct audio *audio, const bool *voice_solo, const bool *voice_muted);

struct synth *audio_get_synth(struct audio *audio);
struct sequencer *audio_get_sequencer(struct audio *audio);

#endif
