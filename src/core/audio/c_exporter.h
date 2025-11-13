#ifndef C_EXPORTER_H
#define C_EXPORTER_H

#include <stdbool.h>
#include <stdint.h>

struct Sequencer;

bool c_exporter_export_to_file(const struct Sequencer *sequencer, const char *filepath);

#endif
