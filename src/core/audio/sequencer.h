#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "synth.h"
#include <stdbool.h>
#include <stdint.h>

struct Note {
	uint32_t time_ms;
	struct NoteParams params;
	bool triggered;
};

#define SEQUENCER_MAX_NOTES 256

struct Sequencer {
	struct Note notes[SEQUENCER_MAX_NOTES];
	uint32_t note_count;
	uint64_t playhead_samples;
	uint32_t sample_rate;
	uint32_t bpm;
	bool playing;
};

void sequencer_init(struct Sequencer *sequencer);
void sequencer_add_note(struct Sequencer *sequencer, uint32_t time_ms, struct NoteParams params);
void sequencer_clear_notes(struct Sequencer *sequencer);
void sequencer_update(
	struct Sequencer *sequencer,
	struct Synth *synth,
	uint32_t samples,
	const bool *voice_solo,
	const bool *voice_muted
);
void sequencer_set_playhead(struct Sequencer *sequencer, uint32_t playhead_ms);
void sequencer_set_bpm(struct Sequencer *sequencer, uint32_t bpm);
void sequencer_play(struct Sequencer *sequencer);
void sequencer_pause(struct Sequencer *sequencer);
void sequencer_stop(struct Sequencer *sequencer);

#endif
