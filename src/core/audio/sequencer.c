#include "sequencer.h"
#include <stdio.h>
#include <string.h>

void sequencer_init(struct Sequencer *sequencer)
{
	memset(sequencer, 0, sizeof(struct Sequencer));
	sequencer->bpm = 120;
	sequencer->playing = false;
}

void sequencer_add_note(struct Sequencer *sequencer, uint32_t time_ms, struct NoteParams params)
{
	if (sequencer->note_count >= SEQUENCER_MAX_NOTES)
	{
		return;
	}

	struct Note *note = &sequencer->notes[sequencer->note_count];
	note->time_ms = time_ms;
	note->params = params;
	note->triggered = false;

	sequencer->note_count++;
}

void sequencer_clear_notes(struct Sequencer *sequencer)
{
	sequencer->note_count = 0;
}

void sequencer_update(struct Sequencer *sequencer, struct Synth *synth, float delta_time_ms)
{
	if (!sequencer->playing)
	{
		return;
	}

	sequencer->playhead_ms += (uint32_t)delta_time_ms;

	bool has_untriggered_notes = false;

	for (uint32_t i = 0; i < sequencer->note_count; i++)
	{
		struct Note *note = &sequencer->notes[i];

		if (!note->triggered && sequencer->playhead_ms >= note->time_ms)
		{
			synth_play_note(synth, note->params);
			note->triggered = true;
		}

		if (!note->triggered)
		{
			has_untriggered_notes = true;
		}
	}

	if (sequencer->note_count > 0 && !has_untriggered_notes)
	{
		sequencer->playing = false;
		sequencer->playhead_ms = 0;

		for (uint32_t i = 0; i < sequencer->note_count; i++)
		{
			sequencer->notes[i].triggered = false;
		}
	}
}

void sequencer_set_playhead(struct Sequencer *sequencer, uint32_t playhead_ms)
{
	sequencer->playhead_ms = playhead_ms;

	for (uint32_t i = 0; i < sequencer->note_count; i++)
	{
		if (sequencer->notes[i].time_ms < playhead_ms)
		{
			sequencer->notes[i].triggered = true;
		}
		else
		{
			sequencer->notes[i].triggered = false;
		}
	}
}

void sequencer_set_bpm(struct Sequencer *sequencer, uint32_t bpm)
{
	sequencer->bpm = bpm;
}

void sequencer_play(struct Sequencer *sequencer)
{
	sequencer->playing = true;
}

void sequencer_pause(struct Sequencer *sequencer)
{
	sequencer->playing = false;
}

void sequencer_stop(struct Sequencer *sequencer)
{
	sequencer->playing = false;
	sequencer_set_playhead(sequencer, 0);
}
