#ifndef C_EXPORTER_H
#define C_EXPORTER_H

#include <stdbool.h>
#include <stdint.h>

struct sequencer;

bool c_exporter_export_to_file(const struct sequencer *sequencer, const char *filepath);

#endif
