#include "button.h"
#include "../../core/graphics/graphics.h"
#include "../../core/graphics/color.h"

#include <stddef.h>

void button_render(struct Graphics *graphics, struct button *button)
{
	if (button == NULL || graphics == NULL)
	{
		return;
	}

	graphics_draw_rounded_rect_outlined(
			graphics, button->x, button->y, button->width, button->height, 6, COLOR_MAGENTA, 2
	);

	int text_x = button->x + (button->width / 2);
	int text_y = button->y + (button->height / 2);
	graphics_draw_text(graphics, button->text, text_x, text_y, 16);
}
