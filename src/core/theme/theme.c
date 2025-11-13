#include "theme.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void theme_init_default(struct theme *theme)
{
	if (theme == NULL)
		return;

	theme->background = color_rgb(42, 46, 64);
	theme->grid_background = color_rgb(28, 30, 46);
	theme->grid_line = color_rgb(65, 69, 89);
	theme->grid_beat_line = color_rgb(81, 87, 109);

	theme->piano_key_white = color_rgb(230, 233, 239);
	theme->piano_key_black = color_rgb(41, 44, 60);
	theme->piano_key_white_text = color_rgb(76, 79, 105);
	theme->piano_key_black_text = color_rgb(198, 208, 245);
	theme->piano_key_separator = color_rgb(81, 87, 109);

	theme->scale_root_grid = color_rgba(140, 170, 238, 40);
	theme->scale_note_grid = color_rgba(186, 187, 241, 30);
	theme->scale_root_piano = color_rgb(140, 170, 238);
	theme->scale_note_piano = color_rgb(186, 187, 241);

	theme->note_default = color_rgb(140, 170, 238);
	theme->note_shadow = color_rgba(0, 0, 0, 60);

	theme->playhead = color_rgba(242, 213, 207, 200);

	theme->statusline_bg = color_rgba(41, 44, 60, 242);
	theme->statusline_text = color_rgb(198, 208, 245);
	theme->statusline_separator = color_rgb(81, 87, 109);
	theme->statusline_bpm = color_rgb(229, 200, 144);
	theme->statusline_highlight_on = color_rgb(166, 209, 137);
	theme->statusline_highlight_off = color_rgb(98, 104, 128);

	theme->voice_colors[0] = color_rgb(239, 159, 118);
	theme->voice_colors[1] = color_rgb(229, 200, 144);
	theme->voice_colors[2] = color_rgb(166, 209, 137);
	theme->voice_colors[3] = color_rgb(140, 170, 238);
	theme->voice_colors[4] = color_rgb(202, 158, 230);
	theme->voice_colors[5] = color_rgb(244, 184, 228);
	theme->voice_colors[6] = color_rgb(231, 130, 132);
	theme->voice_colors[7] = color_rgb(131, 139, 167);
}

static int hex_char_to_int(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	return -1;
}

bool theme_parse_hex_color(const char *hex, struct color *color)
{
	if (hex == NULL || color == NULL)
		return false;

	if (hex[0] == '#')
		hex++;

	size_t len = strlen(hex);
	if (len != 6)
		return false;

	int r1 = hex_char_to_int(hex[0]);
	int r2 = hex_char_to_int(hex[1]);
	int g1 = hex_char_to_int(hex[2]);
	int g2 = hex_char_to_int(hex[3]);
	int b1 = hex_char_to_int(hex[4]);
	int b2 = hex_char_to_int(hex[5]);

	if (r1 < 0 || r2 < 0 || g1 < 0 || g2 < 0 || b1 < 0 || b2 < 0)
		return false;

	color->r = (uint8_t)(r1 * 16 + r2);
	color->g = (uint8_t)(g1 * 16 + g2);
	color->b = (uint8_t)(b1 * 16 + b2);
	color->a = 255;

	return true;
}
