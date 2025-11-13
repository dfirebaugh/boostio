#ifndef SONG_LOADER_H
#define SONG_LOADER_H

#include <stdbool.h>
#include <stdint.h>

struct Audio;

bool song_loader_load_from_file(struct Audio *audio, const char *filepath);

#endif
