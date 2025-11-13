config = {
	window = {
		width = 800,
		height = 600,
		title = "boostio",
		vsync = true,
		resizable = true,
	},

	grid = {
		snap_enabled = true,
		pixels_per_ms = 0.2,
		piano_key_height = 20,
	},

	audio = {
		default_waveform = "square",
		default_duration_ms = 200,
		default_amplitude_dbfs = -6,
		voices = 8,
		sample_rate = 44100,
		buffer_size = 512,
	},

	playback = {
		default_bpm = 120,
		loop_enabled = false,
		loop_start_ms = 0,
		loop_end_ms = 4000,
		follow_playhead = true,
	},

	theme = {
		background = "#1e1e2e",
		foreground = "#cdd6f4",
		grid_line = "#313244",
		note_default = "#89b4fa",
		selection = "#f38ba8",
	},

	plugins = {
		load_list = {
			{
				name = "statusline",
				enabled = true,
				priority = 100,
			},
			{
				name = "test",
				enabled = false,
				priority = 50,
			},
			{
				name = "draw_test",
				enabled = true,
				priority = 50,
			},
		},
	},
}

return config
