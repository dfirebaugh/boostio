#ifndef WAV_EXPORTER_H
#define WAV_EXPORTER_H

#include <stdbool.h>
#include <stdint.h>

struct Sequencer;

bool wav_exporter_export_to_file(const struct Sequencer *sequencer, const char *filepath);

#endif
