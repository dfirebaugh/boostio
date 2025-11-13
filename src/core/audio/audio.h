#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

struct Audio;
struct Synth;
struct Sequencer;

struct Audio *audio_create(void);
void audio_destroy(struct Audio *audio);

bool audio_is_initialized(const struct Audio *audio);

void audio_start(struct Audio *audio);
void audio_stop(struct Audio *audio);

void audio_update(struct Audio *audio);

struct Synth *audio_get_synth(struct Audio *audio);
struct Sequencer *audio_get_sequencer(struct Audio *audio);

#endif
