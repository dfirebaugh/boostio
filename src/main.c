#include "core/graphics/color.h"
#include "core/graphics/graphics.h"
#include "core/graphics/msdf_atlas.h"
#include "core/graphics/window.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	struct WindowConfig config = {
			.width = 800, .height = 600, .title = "Boostio", .resizable = true, .vsync = true
	};

	struct Window *window = window_create(&config);
	if (!window)
	{
		fprintf(stderr, "Failed to create window\n");
		return 1;
	}

	struct Graphics *graphics = graphics_create(window);
	if (!graphics)
	{
		fprintf(stderr, "Failed to create graphics context\n");
		window_destroy(window);
		return 1;
	}

	if (!msdf_atlas_generate(
				"assets/fonts/Noto/NotoSansMNerdFontMono-Regular.ttf",
				"assets/fonts/Noto/noto-atlas.png", "assets/fonts/Noto/noto-atlas.json"
		))
	{
		fprintf(stderr, "Failed to generate font atlas\n");
	}

	graphics_load_font(
			graphics, "assets/fonts/Noto/noto-atlas.json", "assets/fonts/Noto/noto-atlas.png"
	);

	bool running = true;
	while (running)
	{
		running = graphics_poll_events(graphics);

		if (window_is_scancode_pressed(window, SDL_SCANCODE_ESCAPE))
		{
			running = false;
		}

		graphics_clear(graphics, color_rgb(20, 30, 50));

		graphics_set_color(graphics, COLOR_RED);
		graphics_fill_rect(graphics, 100, 100, 200, 150);

		graphics_set_color(graphics, COLOR_GREEN);
		graphics_draw_rect(graphics, 350, 100, 200, 150);

		graphics_set_color(graphics, COLOR_BLUE);
		graphics_draw_line(graphics, 50, 300, 750, 300);

		graphics_set_color(graphics, COLOR_YELLOW);
		graphics_fill_rounded_rect_outlined(graphics, 100, 350, 200, 150, 20, COLOR_RED, 3);

		graphics_set_color(graphics, COLOR_CYAN);
		graphics_draw_rounded_rect_outlined(graphics, 350, 350, 200, 150, 40, COLOR_MAGENTA, 5);

		graphics_set_color(graphics, COLOR_WHITE);
		graphics_draw_text(graphics, "Hello Boostio!", config.width / 2 - 240, 20, 64);
		graphics_draw_text(
				graphics, "Press ESC to quit", config.width / 2 - 120, config.height - 60, 24
		);

		graphics_present(graphics);

		SDL_Delay(16);
	}

	graphics_destroy(graphics);
	window_destroy(window);

	return 0;
}
