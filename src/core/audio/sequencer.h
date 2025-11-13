#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "synth.h"
#include <stdbool.h>
#include <stdint.h>

struct note {
	uint32_t time_ms;
	struct note_params params;
	bool triggered;
};

#define SEQUENCER_MAX_NOTES 256

struct sequencer {
	struct note notes[SEQUENCER_MAX_NOTES];
	uint32_t note_count;
	uint64_t playhead_samples;
	uint32_t sample_rate;
	uint32_t bpm;
	bool playing;
};

void sequencer_init(struct sequencer *sequencer);
void sequencer_add_note(struct sequencer *sequencer, uint32_t time_ms, struct note_params params);
void sequencer_clear_notes(struct sequencer *sequencer);
void sequencer_update(
	struct sequencer *sequencer,
	struct synth *synth,
	uint32_t samples,
	const bool *voice_solo,
	const bool *voice_muted
);
void sequencer_set_playhead(struct sequencer *sequencer, uint32_t playhead_ms);
void sequencer_set_bpm(struct sequencer *sequencer, uint32_t bpm);
void sequencer_play(struct sequencer *sequencer);
void sequencer_pause(struct sequencer *sequencer);
void sequencer_stop(struct sequencer *sequencer);

#endif
