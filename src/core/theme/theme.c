#include "theme.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void theme_init_default(struct theme *theme)
{
	if (theme == NULL)
		return;

	theme->background = (struct rgb_color){42, 46, 64};
	theme->grid_background = (struct rgb_color){28, 30, 46};
	theme->grid_line = (struct rgb_color){65, 69, 89};
	theme->grid_beat_line = (struct rgb_color){81, 87, 109};

	theme->piano_key_white = (struct rgb_color){230, 233, 239};
	theme->piano_key_black = (struct rgb_color){41, 44, 60};
	theme->piano_key_white_text = (struct rgb_color){76, 79, 105};
	theme->piano_key_black_text = (struct rgb_color){198, 208, 245};
	theme->piano_key_separator = (struct rgb_color){81, 87, 109};

	theme->scale_root_grid = (struct rgba_color){140, 170, 238, 40};
	theme->scale_note_grid = (struct rgba_color){186, 187, 241, 30};
	theme->scale_root_piano = (struct rgb_color){140, 170, 238};
	theme->scale_note_piano = (struct rgb_color){186, 187, 241};

	theme->note_default = (struct rgb_color){140, 170, 238};
	theme->note_shadow = (struct rgba_color){0, 0, 0, 60};

	theme->playhead = (struct rgba_color){242, 213, 207, 200};

	theme->statusline_bg = (struct rgba_color){41, 44, 60, 242};
	theme->statusline_text = (struct rgb_color){198, 208, 245};
	theme->statusline_separator = (struct rgb_color){81, 87, 109};
	theme->statusline_bpm = (struct rgb_color){229, 200, 144};
	theme->statusline_highlight_on = (struct rgb_color){166, 209, 137};
	theme->statusline_highlight_off = (struct rgb_color){98, 104, 128};

	theme->voice_colors[0] = (struct rgb_color){239, 159, 118};
	theme->voice_colors[1] = (struct rgb_color){229, 200, 144};
	theme->voice_colors[2] = (struct rgb_color){166, 209, 137};
	theme->voice_colors[3] = (struct rgb_color){140, 170, 238};
	theme->voice_colors[4] = (struct rgb_color){202, 158, 230};
	theme->voice_colors[5] = (struct rgb_color){244, 184, 228};
	theme->voice_colors[6] = (struct rgb_color){231, 130, 132};
	theme->voice_colors[7] = (struct rgb_color){131, 139, 167};
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

bool theme_parse_hex_color(const char *hex, struct rgb_color *color)
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

	return true;
}
