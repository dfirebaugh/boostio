#include "piano_roll.h"
#include "core/audio/scale.h"
#include "core/graphics/color.h"
#include <math.h>
#include <stdio.h>

void piano_roll_ms_to_x(const struct viewport *vp, uint32_t ms, float *out_x)
{
	*out_x = vp->grid_x + ((float)ms - vp->time_offset) * vp->pixels_per_ms;
}

static int32_t piano_key_to_folded_row(uint8_t piano_key, enum scale_type scale, enum root_note root)
{
	int32_t row = 0;
	for (int32_t key = 127; key >= 0; key--)
	{
		if (scale_is_note_in_scale((uint8_t)key, scale, root))
		{
			if (key == piano_key)
			{
				return row;
			}
			row++;
		}
	}
	return -1;
}

void piano_roll_piano_key_to_y(const struct viewport *vp, uint8_t piano_key, float *out_y)
{
	int32_t row = 127 - (int32_t)piano_key;
	int32_t offset_row = row - vp->note_offset;
	*out_y = vp->grid_y + (float)offset_row * vp->piano_key_height;
}

void piano_roll_x_to_ms(const struct viewport *vp, float x, uint32_t *out_ms)
{
	float relative_x = x - vp->grid_x;
	*out_ms = (uint32_t)(vp->time_offset + relative_x / vp->pixels_per_ms);
}

void piano_roll_y_to_piano_key(const struct viewport *vp, float y, uint8_t *out_key)
{
	float relative_y = y - vp->grid_y;
	int32_t offset_row = (int32_t)(relative_y / vp->piano_key_height);
	int32_t row = vp->note_offset + offset_row;
	int32_t piano_key = 127 - row;

	if (piano_key < 0)
		piano_key = 0;
	if (piano_key > 127)
		piano_key = 127;

	*out_key = (uint8_t)piano_key;
}

void piano_roll_get_note_rect(
		const struct viewport *vp, const struct ui_note *note, struct note_rect *out_rect,
		bool fold_mode, enum scale_type scale, enum root_note root
)
{
	piano_roll_ms_to_x(vp, note->ms, &out_rect->x);

	if (fold_mode)
	{
		int32_t folded_row = piano_key_to_folded_row(note->piano_key, scale, root);
		if (folded_row < 0)
		{
			out_rect->y = -1000.0f;
			out_rect->width = 0.0f;
			out_rect->height = 0.0f;
			return;
		}
		int32_t offset_row = folded_row - vp->note_offset;
		out_rect->y = vp->grid_y + (float)offset_row * vp->piano_key_height;
	}
	else
	{
		piano_roll_piano_key_to_y(vp, note->piano_key, &out_rect->y);
	}

	out_rect->width = (float)note->duration_ms * vp->pixels_per_ms;
	out_rect->height = vp->piano_key_height - 2.0f;
}

static bool is_black_key(uint8_t piano_key)
{
	uint8_t note = piano_key % 12;
	return (note == 1 || note == 3 || note == 6 || note == 8 || note == 10);
}

static void get_note_name(uint8_t piano_key, char *out_name, int max_len)
{
	const char *names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	uint8_t note = piano_key % 12;
	int octave = (piano_key / 12) - 1;
	snprintf(out_name, max_len, "%s%d", names[note], octave);
}

void piano_roll_render_grid(struct Graphics *graphics, const struct app_state *state)
{
	const struct viewport *vp = &state->viewport;
	const struct theme *theme = &state->theme;

	graphics_set_color(graphics, color_rgb(theme->grid_background.r, theme->grid_background.g, theme->grid_background.b));
	graphics_fill_rect(graphics, (int)vp->grid_x, (int)vp->grid_y, (int)vp->grid_width, (int)vp->grid_height);

	int32_t visible_keys = (int32_t)(vp->grid_height / vp->piano_key_height) + 2;
	int32_t start_key = 127 - (vp->note_offset + visible_keys);
	int32_t end_key = 127 - vp->note_offset;

	if (start_key < 0)
		start_key = 0;
	if (end_key > 127)
		end_key = 127;

	if (state->fold_mode)
	{
		float row_y = vp->grid_y;
		int32_t current_row = 0;
		for (int32_t key = 127; key >= 0; key--)
		{
			if (!scale_is_note_in_scale((uint8_t)key, state->selected_scale, state->selected_root))
			{
				continue;
			}

			if (current_row < vp->note_offset)
			{
				current_row++;
				continue;
			}

			if (row_y >= vp->grid_y + vp->grid_height)
			{
				break;
			}

			if (scale_is_root_note((uint8_t)key, state->selected_root))
			{
				graphics_set_color(graphics, color_rgba(theme->scale_root_grid.r, theme->scale_root_grid.g, theme->scale_root_grid.b, theme->scale_root_grid.a));
				graphics_fill_rect(graphics, (int)vp->grid_x, (int)row_y, (int)vp->grid_width, (int)vp->piano_key_height);
			}
			else
			{
				graphics_set_color(graphics, color_rgba(theme->scale_note_grid.r, theme->scale_note_grid.g, theme->scale_note_grid.b, theme->scale_note_grid.a));
				graphics_fill_rect(graphics, (int)vp->grid_x, (int)row_y, (int)vp->grid_width, (int)vp->piano_key_height);
			}

			graphics_set_color(graphics, color_rgb(theme->grid_line.r, theme->grid_line.g, theme->grid_line.b));
			graphics_draw_line(graphics, (int)vp->grid_x, (int)row_y, (int)(vp->grid_x + vp->grid_width), (int)row_y);

			row_y += vp->piano_key_height;
			current_row++;
		}
	}
	else
	{
		for (int32_t key = start_key; key <= end_key; key++)
		{
			float y;
			piano_roll_piano_key_to_y(vp, (uint8_t)key, &y);

			if (is_black_key((uint8_t)key))
			{
				graphics_set_color(graphics, color_rgb(theme->piano_key_black.r, theme->piano_key_black.g, theme->piano_key_black.b));
				graphics_fill_rect(graphics, (int)vp->grid_x, (int)y, (int)vp->grid_width, (int)vp->piano_key_height);
			}

			if (state->show_scale_highlights)
			{
				if (scale_is_root_note((uint8_t)key, state->selected_root))
				{
					graphics_set_color(graphics, color_rgba(theme->scale_root_grid.r, theme->scale_root_grid.g, theme->scale_root_grid.b, theme->scale_root_grid.a));
					graphics_fill_rect(graphics, (int)vp->grid_x, (int)y, (int)vp->grid_width, (int)vp->piano_key_height);
				}
				else if (scale_is_note_in_scale((uint8_t)key, state->selected_scale, state->selected_root))
				{
					graphics_set_color(graphics, color_rgba(theme->scale_note_grid.r, theme->scale_note_grid.g, theme->scale_note_grid.b, theme->scale_note_grid.a));
					graphics_fill_rect(graphics, (int)vp->grid_x, (int)y, (int)vp->grid_width, (int)vp->piano_key_height);
				}
			}

			graphics_set_color(graphics, color_rgb(theme->grid_line.r, theme->grid_line.g, theme->grid_line.b));
			graphics_draw_line(graphics, (int)vp->grid_x, (int)y, (int)(vp->grid_x + vp->grid_width), (int)y);
		}
	}

	float ms_per_beat = 60000.0f / (float)state->bpm;
	float ms_per_bar = ms_per_beat * 4.0f;

	uint32_t start_ms = (uint32_t)vp->time_offset;
	uint32_t end_ms = start_ms + (uint32_t)(vp->grid_width / vp->pixels_per_ms);

	if (state->snap_enabled)
	{
		float ms_per_32nd = ms_per_beat / 8.0f;
		uint32_t first_32nd = (uint32_t)(start_ms / ms_per_32nd);
		uint32_t last_32nd = (uint32_t)(end_ms / ms_per_32nd) + 1;

		for (uint32_t note_32nd = first_32nd; note_32nd <= last_32nd; note_32nd++)
		{
			float note_ms_float = (float)note_32nd * ms_per_32nd;
			float x;
			piano_roll_ms_to_x(vp, (uint32_t)note_ms_float, &x);

			if (x >= vp->grid_x && x <= vp->grid_x + vp->grid_width)
			{
				bool is_beat = (note_32nd % 8) == 0;
				bool is_bar = is_beat && ((note_32nd / 8) % 4) == 0;

				if (is_bar)
				{
					uint32_t bar = note_32nd / 32;
					if (bar % 2 == 0)
					{
						graphics_set_color(graphics, color_rgba(theme->grid_line.r, theme->grid_line.g, theme->grid_line.b, 128));
						float next_bar_x;
						piano_roll_ms_to_x(vp, (uint32_t)(note_ms_float + ms_per_bar), &next_bar_x);
						float bar_width = next_bar_x - x;
						graphics_fill_rect(graphics, (int)x, (int)vp->grid_y, (int)bar_width, (int)vp->grid_height);
					}

					graphics_set_color(graphics, color_rgb(theme->grid_beat_line.r, theme->grid_beat_line.g, theme->grid_beat_line.b));
					graphics_draw_line(graphics, (int)x, (int)vp->grid_y, (int)x, (int)(vp->grid_y + vp->grid_height));
				}
				else if (is_beat)
				{
					graphics_set_color(graphics, color_rgba(theme->grid_line.r, theme->grid_line.g, theme->grid_line.b, 200));
					graphics_draw_line(graphics, (int)x, (int)vp->grid_y, (int)x, (int)(vp->grid_y + vp->grid_height));
				}
				else
				{
					graphics_set_color(graphics, color_rgba(theme->grid_line.r, theme->grid_line.g, theme->grid_line.b, 60));
					graphics_draw_line(graphics, (int)x, (int)vp->grid_y, (int)x, (int)(vp->grid_y + vp->grid_height));
				}
			}
		}
	}
	else
	{
		uint32_t first_bar = (uint32_t)(start_ms / ms_per_bar);
		uint32_t last_bar = (uint32_t)(end_ms / ms_per_bar) + 1;

		for (uint32_t bar = first_bar; bar <= last_bar; bar++)
		{
			float bar_ms_float = (float)bar * ms_per_bar;
			float x;
			piano_roll_ms_to_x(vp, (uint32_t)bar_ms_float, &x);

			if (x >= vp->grid_x && x <= vp->grid_x + vp->grid_width)
			{
				if (bar % 2 == 0)
				{
					graphics_set_color(graphics, color_rgba(theme->grid_line.r, theme->grid_line.g, theme->grid_line.b, 128));
					float next_bar_x;
					piano_roll_ms_to_x(vp, (uint32_t)(bar_ms_float + ms_per_bar), &next_bar_x);
					float bar_width = next_bar_x - x;
					graphics_fill_rect(graphics, (int)x, (int)vp->grid_y, (int)bar_width, (int)vp->grid_height);
				}

				graphics_set_color(graphics, color_rgb(theme->grid_beat_line.r, theme->grid_beat_line.g, theme->grid_beat_line.b));
				graphics_draw_line(graphics, (int)x, (int)vp->grid_y, (int)x, (int)(vp->grid_y + vp->grid_height));
			}
		}
	}
}

void piano_roll_render_piano_keys(struct Graphics *graphics, const struct app_state *state)
{
	const struct viewport *vp = &state->viewport;
	const struct theme *theme = &state->theme;
	float piano_width = vp->grid_x;

	graphics_set_color(graphics, color_rgb(theme->background.r, theme->background.g, theme->background.b));
	graphics_fill_rect(graphics, 0, (int)vp->grid_y, (int)piano_width, (int)vp->grid_height);

	int32_t visible_keys = (int32_t)(vp->grid_height / vp->piano_key_height) + 2;
	int32_t start_key = 127 - (vp->note_offset + visible_keys);
	int32_t end_key = 127 - vp->note_offset;

	if (start_key < 0)
		start_key = 0;
	if (end_key > 127)
		end_key = 127;

	if (state->fold_mode)
	{
		float row_y = vp->grid_y;
		int32_t current_row = 0;
		for (int32_t key = 127; key >= 0; key--)
		{
			if (!scale_is_note_in_scale((uint8_t)key, state->selected_scale, state->selected_root))
			{
				continue;
			}

			if (current_row < vp->note_offset)
			{
				current_row++;
				continue;
			}

			if (row_y >= vp->grid_y + vp->grid_height)
			{
				break;
			}

			bool is_root = scale_is_root_note((uint8_t)key, state->selected_root);

			if (is_root)
			{
				graphics_set_color(graphics, color_rgb(theme->scale_root_piano.r, theme->scale_root_piano.g, theme->scale_root_piano.b));
			}
			else
			{
				graphics_set_color(graphics, color_rgb(theme->scale_note_piano.r, theme->scale_note_piano.g, theme->scale_note_piano.b));
			}
			graphics_fill_rect(graphics, 5, (int)row_y + 1, (int)piano_width - 10, (int)vp->piano_key_height - 2);

			graphics_set_color(graphics, color_rgb(theme->piano_key_separator.r, theme->piano_key_separator.g, theme->piano_key_separator.b));
			graphics_draw_line(graphics, 0, (int)row_y, (int)piano_width, (int)row_y);

			if (key % 12 == 0 && vp->piano_key_height >= 15.0f)
			{
				char note_name[8];
				get_note_name((uint8_t)key, note_name, sizeof(note_name));
				graphics_set_color(graphics, color_rgb(theme->piano_key_white_text.r, theme->piano_key_white_text.g, theme->piano_key_white_text.b));
				graphics_draw_text(graphics, note_name, 10, (int)(row_y + vp->piano_key_height / 2.0f + 4.0f), 12);
			}

			row_y += vp->piano_key_height;
			current_row++;
		}
	}
	else
	{
		for (int32_t key = start_key; key <= end_key; key++)
		{
			float y;
			piano_roll_piano_key_to_y(vp, (uint8_t)key, &y);

			bool is_black = is_black_key((uint8_t)key);
			bool is_root = state->show_scale_highlights && scale_is_root_note((uint8_t)key, state->selected_root);
			bool in_scale = state->show_scale_highlights && scale_is_note_in_scale((uint8_t)key, state->selected_scale, state->selected_root);

			if (is_black)
			{
				graphics_set_color(graphics, color_rgb(theme->piano_key_black.r, theme->piano_key_black.g, theme->piano_key_black.b));
			}
			else
			{
				graphics_set_color(graphics, color_rgb(theme->piano_key_white.r, theme->piano_key_white.g, theme->piano_key_white.b));
			}
			graphics_fill_rect(graphics, 5, (int)y + 1, (int)piano_width - 10, (int)vp->piano_key_height - 2);

			if (is_root)
			{
				graphics_set_color(graphics, color_rgba(theme->scale_root_piano.r, theme->scale_root_piano.g, theme->scale_root_piano.b, 180));
				graphics_fill_rect(graphics, 5, (int)y + 1, (int)piano_width - 10, (int)vp->piano_key_height - 2);
			}
			else if (in_scale)
			{
				graphics_set_color(graphics, color_rgba(theme->scale_note_piano.r, theme->scale_note_piano.g, theme->scale_note_piano.b, 140));
				graphics_fill_rect(graphics, 5, (int)y + 1, (int)piano_width - 10, (int)vp->piano_key_height - 2);
			}

			graphics_set_color(graphics, color_rgb(theme->piano_key_separator.r, theme->piano_key_separator.g, theme->piano_key_separator.b));
			graphics_draw_line(graphics, 0, (int)y, (int)piano_width, (int)y);

			if (key % 12 == 0 && vp->piano_key_height >= 15.0f)
			{
				char note_name[8];
				get_note_name((uint8_t)key, note_name, sizeof(note_name));
				if (is_black_key((uint8_t)key))
				{
					graphics_set_color(graphics, color_rgb(theme->piano_key_black_text.r, theme->piano_key_black_text.g, theme->piano_key_black_text.b));
				}
				else
				{
					graphics_set_color(graphics, color_rgb(theme->piano_key_white_text.r, theme->piano_key_white_text.g, theme->piano_key_white_text.b));
				}
				graphics_draw_text(graphics, note_name, 10, (int)(y + vp->piano_key_height / 2.0f + 4.0f), 12);
			}
		}
	}
}

void piano_roll_render_notes(struct Graphics *graphics, const struct app_state *state)
{
	const struct viewport *vp = &state->viewport;
	const struct theme *theme = &state->theme;

	for (uint32_t i = 0; i < state->note_count; i++)
	{
		const struct ui_note *note = &state->notes[i];
		struct note_rect rect;
		piano_roll_get_note_rect(vp, note, &rect, state->fold_mode, state->selected_scale, state->selected_root);

		if (rect.x + rect.width < vp->grid_x || rect.x > vp->grid_x + vp->grid_width)
		{
			continue;
		}

		if (rect.y + rect.height < vp->grid_y || rect.y > vp->grid_y + vp->grid_height)
		{
			continue;
		}

		struct rgb_color voice_color = theme->voice_colors[note->voice % 8];
		struct Color note_color = {voice_color.r, voice_color.g, voice_color.b, 255};

		bool is_selected = false;
		for (uint32_t j = 0; j < state->selection.count; j++)
		{
			if (state->selection.selected_ids[j] == note->id)
			{
				is_selected = true;
				break;
			}
		}

		if (is_selected)
		{
			note_color.r = (uint8_t)fminf(255, note_color.r * 1.3f);
			note_color.g = (uint8_t)fminf(255, note_color.g * 1.3f);
			note_color.b = (uint8_t)fminf(255, note_color.b * 1.3f);
		}

		graphics_set_color(graphics, note_color);
		graphics_fill_rounded_rect(graphics, (int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, 3);

		if (is_selected)
		{
			graphics_set_color(graphics, COLOR_WHITE);
			graphics_draw_rounded_rect(graphics, (int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, 3);
		}

		graphics_set_color(graphics, color_rgba(theme->note_shadow.r, theme->note_shadow.g, theme->note_shadow.b, theme->note_shadow.a));
		graphics_fill_rect(graphics, (int)rect.x + 2, (int)(rect.y + rect.height - 4), (int)(rect.width - 4), 3);
	}
}

void piano_roll_render_playhead(struct Graphics *graphics, const struct app_state *state)
{
	const struct viewport *vp = &state->viewport;
	const struct theme *theme = &state->theme;
	float x;
	piano_roll_ms_to_x(vp, state->playhead_ms, &x);

	if (x >= vp->grid_x && x <= vp->grid_x + vp->grid_width)
	{
		graphics_set_color(graphics, color_rgba(theme->playhead.r, theme->playhead.g, theme->playhead.b, theme->playhead.a));
		graphics_fill_rect(graphics, (int)(x - 1), (int)(vp->grid_y), 1, state->window_height);

		graphics_fill_rect(graphics, (int)(x - 5), (int)(vp->grid_y - 10), 10, 10);
	}
}

void piano_roll_render(struct Graphics *graphics, const struct app_state *state)
{
	piano_roll_render_grid(graphics, state);
	piano_roll_render_notes(graphics, state);
	piano_roll_render_playhead(graphics, state);
	piano_roll_render_piano_keys(graphics, state);
}
