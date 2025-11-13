#ifndef SONG_LOADER_H
#define SONG_LOADER_H

#include <stdbool.h>
#include <stdint.h>

struct Audio;
struct app_state;

bool song_loader_load_from_file(struct Audio *audio, struct app_state *state, const char *filepath);

#endif
