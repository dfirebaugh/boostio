#include "viewport_utils.h"
#include "app_state.h"

void viewport_ms_to_x(const struct viewport *vp, uint32_t ms, float *out_x)
{
	*out_x = vp->grid_x + ((float)ms - vp->time_offset) * vp->pixels_per_ms;
}

void viewport_x_to_ms(const struct viewport *vp, float x, uint32_t *out_ms)
{
	float relative_x = x - vp->grid_x;
	*out_ms = (uint32_t)(vp->time_offset + relative_x / vp->pixels_per_ms);
}

void viewport_piano_key_to_y(const struct viewport *vp, uint8_t piano_key, float *out_y)
{
	int32_t row = 99 - (int32_t)piano_key;
	int32_t offset_row = row - vp->note_offset;
	*out_y = vp->grid_y + (float)offset_row * vp->piano_key_height;
}

void viewport_y_to_piano_key(const struct viewport *vp, float y, uint8_t *out_key)
{
	float relative_y = y - vp->grid_y;
	int32_t offset_row = (int32_t)(relative_y / vp->piano_key_height);
	int32_t row = vp->note_offset + offset_row;
	int32_t piano_key = 99 - row;

	if (piano_key < 45)
		piano_key = 45;
	if (piano_key > 99)
		piano_key = 99;

	*out_key = (uint8_t)piano_key;
}
