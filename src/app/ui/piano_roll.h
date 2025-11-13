#ifndef BOOSTIO_PIANO_ROLL_H
#define BOOSTIO_PIANO_ROLL_H

#include "app/app_state.h"
#include "core/graphics/graphics.h"

struct note_rect
{
	float x;
	float y;
	float width;
	float height;
};

void piano_roll_ms_to_x(const struct viewport *vp, uint32_t ms, float *out_x);

void piano_roll_piano_key_to_y(const struct viewport *vp, uint8_t piano_key, float *out_y);

void piano_roll_x_to_ms(const struct viewport *vp, float x, uint32_t *out_ms);

void piano_roll_y_to_piano_key(const struct viewport *vp, float y, uint8_t *out_key);

void piano_roll_get_note_rect(
		const struct viewport *vp, const struct ui_note *note, struct note_rect *out_rect,
		bool fold_mode, enum scale_type scale, enum root_note root
);

void piano_roll_render_grid(struct Graphics *graphics, const struct app_state *state);

void piano_roll_render_piano_keys(struct Graphics *graphics, const struct app_state *state);

void piano_roll_render_notes(struct Graphics *graphics, const struct app_state *state);

void piano_roll_render_playhead(struct Graphics *graphics, const struct app_state *state);

void piano_roll_render(struct Graphics *graphics, const struct app_state *state);

#endif
