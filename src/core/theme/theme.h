#ifndef BOOSTIO_THEME_H
#define BOOSTIO_THEME_H

#include <stdbool.h>

#include "core/graphics/color.h"

struct theme {
	struct color background;
	struct color grid_background;
	struct color grid_line;
	struct color grid_beat_line;

	struct color piano_key_white;
	struct color piano_key_black;
	struct color piano_key_white_text;
	struct color piano_key_black_text;
	struct color piano_key_separator;

	struct color scale_root_grid;
	struct color scale_note_grid;
	struct color scale_root_piano;
	struct color scale_note_piano;

	struct color note_default;
	struct color note_shadow;

	struct color playhead;

	struct color statusline_bg;
	struct color statusline_text;
	struct color statusline_separator;
	struct color statusline_bpm;
	struct color statusline_highlight_on;
	struct color statusline_highlight_off;

	struct color voice_colors[8];
};

void theme_init_default(struct theme *theme);

bool theme_parse_hex_color(const char *hex, struct color *color);

#endif
