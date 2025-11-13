#ifndef SCALE_H
#define SCALE_H

#include <stdbool.h>
#include <stdint.h>

enum scale_type {
	SCALE_CHROMATIC,
	SCALE_MAJOR,
	SCALE_MINOR,
	SCALE_HARMONIC_MINOR,
	SCALE_MELODIC_MINOR,
	SCALE_PENTATONIC_MAJOR,
	SCALE_PENTATONIC_MINOR,
	SCALE_BLUES,
	SCALE_DORIAN,
	SCALE_PHRYGIAN,
	SCALE_LYDIAN,
	SCALE_MIXOLYDIAN,
	SCALE_AEOLIAN,
	SCALE_LOCRIAN,
	SCALE_TYPE_COUNT
};

enum root_note {
	ROOT_C = 0,
	ROOT_CS = 1,
	ROOT_D = 2,
	ROOT_DS = 3,
	ROOT_E = 4,
	ROOT_F = 5,
	ROOT_FS = 6,
	ROOT_G = 7,
	ROOT_GS = 8,
	ROOT_A = 9,
	ROOT_AS = 10,
	ROOT_B = 11
};

struct scale_info {
	const uint8_t *intervals;
	uint8_t interval_count;
};

const struct scale_info *scale_get_info(enum scale_type type);

const char *scale_type_to_string(enum scale_type type);
const char *root_note_to_string(enum root_note root);

enum scale_type scale_type_from_string(const char *str);
enum root_note root_note_from_string(const char *str);

bool scale_is_note_in_scale(uint8_t piano_key, enum scale_type type, enum root_note root);

bool scale_is_root_note(uint8_t piano_key, enum root_note root);

#endif
