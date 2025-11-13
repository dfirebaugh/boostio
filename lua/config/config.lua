local theme = require("theme")
require("keybindings")

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

	playback = {
		default_bpm = 120,
		loop_enabled = false,
		loop_start_ms = 0,
		loop_end_ms = 4000,
		follow_playhead = true,
	},

	theme = theme,

	plugins = {
		load_list = {
			{
				name = "piano_roll",
				enabled = true,
				priority = 1,
				options = {
					-- Interaction settings
					-- edge_threshold = 5,              -- Pixels from note edge to trigger resize
					-- drag_threshold = 5,              -- Pixels to move before drag starts
					-- double_click_threshold_ms = 500, -- Max time between clicks for double-click

					-- Note settings
					-- min_note_duration_ms = 10,       -- Minimum duration when resizing notes
					-- default_note_duration_ms = 500,  -- Default duration for new notes

					-- Piano key range
					-- piano_key_min = 45,              -- Lowest piano key to display (A2)
					-- piano_key_max = 99,              -- Highest piano key to display (D#7)
				},
			},
			{
				name = "toast",
				enabled = true,
				priority = 200,
			},
			{
				name = "fps_display",
				enabled = true,
				priority = 210,
			},
			{
				name = "statusline",
				enabled = true,
				priority = 100,
			},
			{
				name = "playback_controls",
				enabled = true,
				priority = 90,
			},
			{
				name = "voice_controls",
				enabled = true,
				priority = 85,
			},
			{
				name = "instrument_selector",
				enabled = true,
				priority = 80,
			},
			{
				name = "voice_validation",
				enabled = true,
				priority = 55,
			},
			{
				name = "splash_screen",
				enabled = true,
				priority = 300,
				options = {
					duration_ms = 200,
					fade_in_ms = 20,
					fade_out_ms = 100,
				},
			},
		},
	},
}

return config
