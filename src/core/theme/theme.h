#ifndef BOOSTIO_THEME_H
#define BOOSTIO_THEME_H

#include <stdint.h>

struct rgb_color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct rgba_color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct theme
{
	struct rgb_color background;
	struct rgb_color grid_background;
	struct rgb_color grid_line;
	struct rgb_color grid_beat_line;

	struct rgb_color piano_key_white;
	struct rgb_color piano_key_black;
	struct rgb_color piano_key_white_text;
	struct rgb_color piano_key_black_text;
	struct rgb_color piano_key_separator;

	struct rgba_color scale_root_grid;
	struct rgba_color scale_note_grid;
	struct rgb_color scale_root_piano;
	struct rgb_color scale_note_piano;

	struct rgb_color note_default;
	struct rgba_color note_shadow;

	struct rgba_color playhead;

	struct rgba_color statusline_bg;
	struct rgb_color statusline_text;
	struct rgb_color statusline_separator;
	struct rgb_color statusline_bpm;
	struct rgb_color statusline_highlight_on;
	struct rgb_color statusline_highlight_off;

	struct rgb_color voice_colors[8];
};

void theme_init_default(struct theme *theme);

bool theme_parse_hex_color(const char *hex, struct rgb_color *color);

#endif
