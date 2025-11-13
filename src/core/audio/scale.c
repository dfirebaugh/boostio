#include "scale.h"

#include <stddef.h>
#include <string.h>

static const uint8_t scale_chromatic[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
static const uint8_t scale_major[] = {0, 2, 4, 5, 7, 9, 11};
static const uint8_t scale_minor[] = {0, 2, 3, 5, 7, 8, 10};
static const uint8_t scale_harmonic_minor[] = {0, 2, 3, 5, 7, 8, 11};
static const uint8_t scale_melodic_minor[] = {0, 2, 3, 5, 7, 9, 11};
static const uint8_t scale_pentatonic_major[] = {0, 2, 4, 7, 9};
static const uint8_t scale_pentatonic_minor[] = {0, 3, 5, 7, 10};
static const uint8_t scale_blues[] = {0, 3, 5, 6, 7, 10};
static const uint8_t scale_dorian[] = {0, 2, 3, 5, 7, 9, 10};
static const uint8_t scale_phrygian[] = {0, 1, 3, 5, 7, 8, 10};
static const uint8_t scale_lydian[] = {0, 2, 4, 6, 7, 9, 11};
static const uint8_t scale_mixolydian[] = {0, 2, 4, 5, 7, 9, 10};
static const uint8_t scale_aeolian[] = {0, 2, 3, 5, 7, 8, 10};
static const uint8_t scale_locrian[] = {0, 1, 3, 5, 6, 8, 10};

static const struct scale_info scale_infos[] = {
	[SCALE_CHROMATIC] = {scale_chromatic, 12},
	[SCALE_MAJOR] = {scale_major, 7},
	[SCALE_MINOR] = {scale_minor, 7},
	[SCALE_HARMONIC_MINOR] = {scale_harmonic_minor, 7},
	[SCALE_MELODIC_MINOR] = {scale_melodic_minor, 7},
	[SCALE_PENTATONIC_MAJOR] = {scale_pentatonic_major, 5},
	[SCALE_PENTATONIC_MINOR] = {scale_pentatonic_minor, 5},
	[SCALE_BLUES] = {scale_blues, 6},
	[SCALE_DORIAN] = {scale_dorian, 7},
	[SCALE_PHRYGIAN] = {scale_phrygian, 7},
	[SCALE_LYDIAN] = {scale_lydian, 7},
	[SCALE_MIXOLYDIAN] = {scale_mixolydian, 7},
	[SCALE_AEOLIAN] = {scale_aeolian, 7},
	[SCALE_LOCRIAN] = {scale_locrian, 7},
};

static const char *scale_type_names[] = {
	[SCALE_CHROMATIC] = "chromatic",
	[SCALE_MAJOR] = "major",
	[SCALE_MINOR] = "minor",
	[SCALE_HARMONIC_MINOR] = "harmonic_minor",
	[SCALE_MELODIC_MINOR] = "melodic_minor",
	[SCALE_PENTATONIC_MAJOR] = "pentatonic_major",
	[SCALE_PENTATONIC_MINOR] = "pentatonic_minor",
	[SCALE_BLUES] = "blues",
	[SCALE_DORIAN] = "dorian",
	[SCALE_PHRYGIAN] = "phrygian",
	[SCALE_LYDIAN] = "lydian",
	[SCALE_MIXOLYDIAN] = "mixolydian",
	[SCALE_AEOLIAN] = "aeolian",
	[SCALE_LOCRIAN] = "locrian",
};

static const char *root_note_names[] = {
	[ROOT_C] = "C",
	[ROOT_CS] = "C#",
	[ROOT_D] = "D",
	[ROOT_DS] = "D#",
	[ROOT_E] = "E",
	[ROOT_F] = "F",
	[ROOT_FS] = "F#",
	[ROOT_G] = "G",
	[ROOT_GS] = "G#",
	[ROOT_A] = "A",
	[ROOT_AS] = "A#",
	[ROOT_B] = "B",
};

const struct scale_info *scale_get_info(enum scale_type type)
{
	if (type >= SCALE_TYPE_COUNT) {
		return NULL;
	}
	return &scale_infos[type];
}

const char *scale_type_to_string(enum scale_type type)
{
	if (type >= SCALE_TYPE_COUNT) {
		return "unknown";
	}
	return scale_type_names[type];
}

const char *root_note_to_string(enum root_note root)
{
	if (root > ROOT_B) {
		return "unknown";
	}
	return root_note_names[root];
}

enum scale_type scale_type_from_string(const char *str)
{
	if (str == NULL) {
		return SCALE_PENTATONIC_MINOR;
	}

	for (int i = 0; i < SCALE_TYPE_COUNT; i++) {
		if (strcmp(str, scale_type_names[i]) == 0) {
			return (enum scale_type)i;
		}
	}

	return SCALE_PENTATONIC_MINOR;
}

enum root_note root_note_from_string(const char *str)
{
	if (str == NULL) {
		return ROOT_C;
	}

	for (int i = ROOT_C; i <= ROOT_B; i++) {
		if (strcmp(str, root_note_names[i]) == 0) {
			return (enum root_note)i;
		}
	}

	return ROOT_C;
}

bool scale_is_note_in_scale(uint8_t piano_key, enum scale_type type, enum root_note root)
{
	const struct scale_info *info = scale_get_info(type);
	if (info == NULL) {
		return false;
	}

	uint8_t note_in_octave = piano_key % 12;
	uint8_t root_semitone = (uint8_t)root;
	uint8_t relative_note = (note_in_octave + 12 - root_semitone) % 12;

	for (uint8_t i = 0; i < info->interval_count; i++) {
		if (info->intervals[i] == relative_note) {
			return true;
		}
	}

	return false;
}

bool scale_is_root_note(uint8_t piano_key, enum root_note root)
{
	uint8_t note_in_octave = piano_key % 12;
	return note_in_octave == (uint8_t)root;
}
