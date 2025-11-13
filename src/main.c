#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "app_controller.h"
#include "app_state.h"
#include "audio.h"
#include "graphics.h"
#include "msdf_atlas.h"
#include "platform.h"
#include "sequencer.h"
#include "song_loader.h"
#include "window.h"

int main(int argc, char *argv[])
{
	struct platform_paths paths;
	platform_paths_init(&paths);

	if (!platform_ensure_directory(paths.data_dir)) {
		fprintf(stderr, "Failed to create data directory: %s\n", paths.data_dir);
		platform_paths_free(&paths);
		return 1;
	}

	if (!platform_ensure_directory(paths.config_dir)) {
		fprintf(stderr, "Failed to create config directory: %s\n", paths.config_dir);
		platform_paths_free(&paths);
		return 1;
	}

	struct window_config config = {
		.width = 800, .height = 600, .title = "Boostio", .resizable = true, .vsync = true
	};

	struct window *window = window_create(&config);
	if (!window) {
		fprintf(stderr, "Failed to create window\n");
		return 1;
	}

	struct graphics *graphics = graphics_create(window);
	if (!graphics) {
		fprintf(stderr, "Failed to create graphics context\n");
		window_destroy(window);
		return 1;
	}

	struct audio *audio = audio_create();
	if (!audio) {
		fprintf(stderr, "Failed to create audio system\n");
		graphics_destroy(graphics);
		window_destroy(window);
		return 1;
	}

	audio_start(audio);

	char atlas_png_path[512];
	char atlas_json_path[512];
	snprintf(atlas_png_path, sizeof(atlas_png_path), "%s/noto-atlas.png", paths.data_dir);
	snprintf(atlas_json_path, sizeof(atlas_json_path), "%s/noto-atlas.json", paths.data_dir);

	if (!msdf_atlas_generate(
		    "assets/fonts/Noto/NotoSansMNerdFontMono-Regular.ttf",
		    atlas_png_path,
		    atlas_json_path
	    )) {
		fprintf(stderr, "Failed to generate font atlas\n");
	}

	if (!graphics_load_font(graphics, atlas_json_path, atlas_png_path)) {
		fprintf(stderr, "Failed to load font atlas from %s\n", atlas_json_path);
		audio_destroy(audio);
		graphics_destroy(graphics);
		window_destroy(window);
		platform_paths_free(&paths);
		return 1;
	}

	struct app_controller controller;
	if (!app_controller_init(&controller, graphics, audio, &paths)) {
		fprintf(stderr, "Failed to initialize app controller\n");
		audio_destroy(audio);
		graphics_destroy(graphics);
		window_destroy(window);
		platform_paths_free(&paths);
		return 1;
	}

	char config_path[512];
	snprintf(config_path, sizeof(config_path), "%s/config.lua", paths.config_dir);

	if (!app_controller_init_lua(&controller, config_path)) {
		fprintf(stderr, "Warning: Failed to initialize Lua system from %s\n", config_path);
	}

	if (argc > 1) {
		const char *song_path = argv[1];
		printf("Loading song from: %s\n", song_path);
		strncpy(controller.state.current_file_path, song_path, 511);
		controller.state.current_file_path[511] = '\0';
		printf("Set current file path: %s\n", song_path);

		if (song_loader_load_from_file(audio, &controller.state, song_path)) {
			printf("Song loaded successfully\n");

			struct sequencer *sequencer = audio_get_sequencer(audio);
			app_state_sync_notes_from_sequencer(&controller.state, sequencer);
			printf("Synced %d notes to UI\n", controller.state.note_count);
		} else {
			fprintf(stderr, "Failed to load song from %s, starting with empty song\n", song_path);
		}
	}

	const double target_frame_time = 1.0 / 60.0;
	uint64_t frequency = SDL_GetPerformanceFrequency();
	uint64_t frame_start = SDL_GetPerformanceCounter();
	double delta_time = target_frame_time;

	while (app_controller_is_running(&controller)) {
		uint64_t frame_end = SDL_GetPerformanceCounter();
		delta_time = (double)(frame_end - frame_start) / (double)frequency;
		frame_start = frame_end;

		if (!graphics_poll_events(graphics)) {
			app_controller_stop(&controller);
		}

		app_controller_update(&controller, (float)delta_time);
		audio_update(audio, controller.state.voice_solo, controller.state.voice_muted);
		app_controller_render(&controller);

		graphics_present(graphics);

		uint64_t frame_work_end = SDL_GetPerformanceCounter();
		double frame_work_time = (double)(frame_work_end - frame_start) / (double)frequency;

		if (frame_work_time < target_frame_time) {
			double delay_time = target_frame_time - frame_work_time;
			SDL_Delay((uint32_t)(delay_time * 1000.0));
		}
	}

	app_controller_deinit(&controller);
	audio_destroy(audio);
	graphics_destroy(graphics);
	window_destroy(window);
	platform_paths_free(&paths);

	return 0;
}
