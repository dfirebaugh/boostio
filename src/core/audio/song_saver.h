#ifndef SONG_SAVER_H
#define SONG_SAVER_H

#include <stdbool.h>
#include <stdint.h>

struct app_state;
struct Sequencer;

bool song_saver_save_to_file(
	const struct app_state *state, const struct Sequencer *sequencer, const char *filepath
);

#endif
