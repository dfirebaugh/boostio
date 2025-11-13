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

	theme = {
		-- Catppuccin Frappe color palette
		palette = {
			base = "#303446",
			surface0 = "#414559",
			surface1 = "#51576d",
			surface2 = "#626880",
			overlay0 = "#737994",
			overlay1 = "#838ba7",
			overlay2 = "#949cbb",
			text = "#c6d0f5",
			subtext1 = "#b5bfe2",
			subtext0 = "#a5adce",
			rosewater = "#f2d5cf",
			flamingo = "#eebebe",
			pink = "#f4b8e4",
			mauve = "#ca9ee6",
			red = "#e78284",
			maroon = "#ea999c",
			peach = "#ef9f76",
			yellow = "#e5c890",
			green = "#a6d189",
			teal = "#81c8be",
			sky = "#99d1db",
			sapphire = "#85c1dc",
			blue = "#8caaee",
			lavender = "#babbf1",
		},

		-- UI element colors
		background = "#303446",
		grid_background = "#1e2030",
		grid_line = "#414559",
		grid_beat_line = "#51576d",

		-- Piano roll colors
		piano_key_white = "#e6e9ef",
		piano_key_black = "#292c3c",
		piano_key_white_text = "#4c4f69",
		piano_key_black_text = "#c6d0f5",
		piano_key_separator = "#51576d",

		-- Scale highlighting
		scale_root_grid = "#8caaee",
		scale_root_grid_alpha = 40,
		scale_note_grid = "#babbf1",
		scale_note_grid_alpha = 30,
		scale_root_piano = "#8caaee",
		scale_note_piano = "#babbf1",

		-- Notes (will use voice colors but this is fallback)
		note_default = "#8caaee",
		note_shadow = "#000000",
		note_shadow_alpha = 60,

		-- Playhead
		playhead = "#f2d5cf",
		playhead_alpha = 200,

		-- Statusline colors
		statusline_bg = "#292c3c",
		statusline_bg_alpha = 0.95,
		statusline_text = "#c6d0f5",
		statusline_separator = "#51576d",
		statusline_bpm = "#e5c890",
		statusline_highlight_on = "#a6d189",
		statusline_highlight_off = "#626880",

		-- Voice colors (Catppuccin Frappe inspired)
		voice_colors = {
			"#ef9f76", -- peach
			"#e5c890", -- yellow
			"#a6d189", -- green
			"#8caaee", -- blue
			"#ca9ee6", -- mauve
			"#f4b8e4", -- pink
			"#e78284", -- red
			"#838ba7", -- overlay1
		},
	},

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
		},
	},
}

return config
