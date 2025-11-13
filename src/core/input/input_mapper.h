#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

#include "input_types.h"

#include <stdbool.h>
#include <stdint.h>

bool input_mapper_scancode_to_key(uint32_t scancode, enum key *out_key);

#endif
