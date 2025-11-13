local rendering = {}

local function draw_background(vp, theme)
	local bg = boostio.hexToRgb(theme.grid_background)
	boostio.drawRectangle(vp.grid_x, vp.grid_y, vp.grid_width, vp.grid_height, bg.r, bg.g, bg.b, 1.0)
end

local function get_visible_key_range(vp, options)
	local visible_keys = math.floor(vp.grid_height / vp.piano_key_height) + 2
	local start_key = options.piano_key_max - (vp.note_offset + visible_keys)
	local end_key = options.piano_key_max - vp.note_offset

	if start_key < options.piano_key_min then
		start_key = options.piano_key_min
	end
	if end_key > options.piano_key_max then
		end_key = options.piano_key_max
	end

	return start_key, end_key
end

local function draw_scale_highlight(vp, y, height, key, state, theme)
	if not state.show_scale_highlights then
		return
	end

	if boostio.isRootNote(key, state.selected_root) then
		local color = boostio.hexToRgb(theme.scale_root_grid)
		boostio.drawRectangle(vp.grid_x, y, vp.grid_width, height, color.r, color.g, color.b, theme.scale_root_grid_alpha)
		return
	end

	if not boostio or not boostio.isNoteInScale then
		return
	end

	local success, in_scale = pcall(boostio.isNoteInScale, key, state.selected_scale, state.selected_root)
	if success and in_scale then
		local color = boostio.hexToRgb(theme.scale_note_grid)
		boostio.drawRectangle(vp.grid_x, y, vp.grid_width, height, color.r, color.g, color.b, theme.scale_note_grid_alpha)
	end
end

local function draw_folded_grid_row(vp, row_y, key, state, theme)
	draw_scale_highlight(vp, row_y, vp.piano_key_height, key, state, theme)

	local line_color = boostio.hexToRgb(theme.grid_line)
	boostio.drawLine(vp.grid_x, row_y, vp.grid_x + vp.grid_width, row_y, line_color.r, line_color.g, line_color.b, 1.0)
end

local function draw_folded_grid(vp, state, theme, options)
	if not boostio or not boostio.isNoteInScale then
		return
	end

	local row_y = vp.grid_y
	local current_row = 0

	for key = options.piano_key_max, options.piano_key_min, -1 do
		local success, in_scale = pcall(boostio.isNoteInScale, key, state.selected_scale, state.selected_root)
		if success and in_scale then
			if current_row >= vp.note_offset then
				if row_y >= vp.grid_y + vp.grid_height then
					break
				end

				draw_folded_grid_row(vp, row_y, key, state, theme)
				row_y = row_y + vp.piano_key_height
			end
			current_row = current_row + 1
		end
	end
end

local function draw_black_key_background(vp, y, theme, utils)
	local color = boostio.hexToRgb(theme.piano_key_black)
	boostio.drawRectangle(vp.grid_x, y, vp.grid_width, vp.piano_key_height, color.r, color.g, color.b, 1.0)
end

local function draw_unfolded_grid_row(vp, key, y, state, theme, utils)
	if utils.is_black_key(key) then
		draw_black_key_background(vp, y, theme, utils)
	end

	draw_scale_highlight(vp, y, vp.piano_key_height, key, state, theme)

	local line_color = boostio.hexToRgb(theme.grid_line)
	boostio.drawLine(vp.grid_x, y, vp.grid_x + vp.grid_width, y, line_color.r, line_color.g, line_color.b, 1.0)
end

local function draw_unfolded_grid(vp, state, theme, options, utils)
	local start_key, end_key = get_visible_key_range(vp, options)

	for key = start_key, end_key do
		local y = boostio.pianoKeyToY(key)
		draw_unfolded_grid_row(vp, key, y, state, theme, utils)
	end
end

local function draw_grid_lines(vp, state, theme)
	local ms_per_beat = 60000.0 / state.bpm
	local ms_per_bar = ms_per_beat * 4.0

	local start_ms = vp.time_offset
	local end_ms = start_ms + (vp.grid_width / vp.pixels_per_ms)

	if state.snap_enabled then
		draw_snap_grid_lines(vp, state, theme, ms_per_beat, ms_per_bar, start_ms, end_ms)
		return
	end

	draw_bar_lines(vp, theme, ms_per_bar, start_ms, end_ms)
end

local function draw_bar_rectangle_if_even(x, vp, bar, theme, ms_per_bar)
	if bar % 2 ~= 0 then
		return
	end

	local color = boostio.hexToRgb(theme.grid_line)
	boostio.drawRectangle(x, vp.grid_y, ms_per_bar * vp.pixels_per_ms, vp.grid_height, color.r, color.g, color.b, 0.5)
end

local function draw_bar_line(x, vp, theme)
	local beat_color = boostio.hexToRgb(theme.grid_beat_line)
	boostio.drawLine(x, vp.grid_y, x, vp.grid_y + vp.grid_height, beat_color.r, beat_color.g, beat_color.b, 1.0)
end

function draw_bar_lines(vp, theme, ms_per_bar, start_ms, end_ms)
	local first_bar = math.floor(start_ms / ms_per_bar)
	local last_bar = math.floor(end_ms / ms_per_bar) + 1

	for bar = first_bar, last_bar do
		local bar_ms_float = bar * ms_per_bar
		local x = boostio.msToX(bar_ms_float)

		if x >= vp.grid_x and x <= vp.grid_x + vp.grid_width then
			draw_bar_rectangle_if_even(x, vp, bar, theme, ms_per_bar)
			draw_bar_line(x, vp, theme)
		end
	end
end

local function draw_snap_grid_line(x, vp, theme, is_bar, is_beat, bar, ms_per_bar)
	if is_bar then
		draw_bar_rectangle_if_even(x, vp, bar, theme, ms_per_bar)
		draw_bar_line(x, vp, theme)
		return
	end

	local color = boostio.hexToRgb(theme.grid_line)
	if is_beat then
		boostio.drawLine(x, vp.grid_y, x, vp.grid_y + vp.grid_height, color.r, color.g, color.b, 0.78)
		return
	end

	boostio.drawLine(x, vp.grid_y, x, vp.grid_y + vp.grid_height, color.r, color.g, color.b, 0.23)
end

function draw_snap_grid_lines(vp, state, theme, ms_per_beat, ms_per_bar, start_ms, end_ms)
	local ms_per_32nd = ms_per_beat / 8.0
	local first_32nd = math.floor(start_ms / ms_per_32nd)
	local last_32nd = math.floor(end_ms / ms_per_32nd) + 1

	for note_32nd = first_32nd, last_32nd do
		local note_ms_float = note_32nd * ms_per_32nd
		local x = boostio.msToX(note_ms_float)

		if x >= vp.grid_x and x <= vp.grid_x + vp.grid_width then
			local is_beat = (note_32nd % 8) == 0
			local is_bar = is_beat and ((math.floor(note_32nd / 8) % 4) == 0)
			local bar = math.floor(note_32nd / 32)

			draw_snap_grid_line(x, vp, theme, is_bar, is_beat, bar, ms_per_bar)
		end
	end
end

function rendering.render_grid(state, theme, options, utils)
	local vp = state.viewport

	draw_background(vp, theme)

	if state.fold_mode then
		draw_folded_grid(vp, state, theme, options)
	else
		draw_unfolded_grid(vp, state, theme, options, utils)
	end

	draw_grid_lines(vp, state, theme)
end

local function draw_piano_key_background(y, piano_width, vp, theme)
	local bg = boostio.hexToRgb(theme.background)
	boostio.drawRectangle(0, vp.grid_y, piano_width, vp.grid_height, bg.r, bg.g, bg.b, 1.0)
end

local function draw_piano_key_rect(y, piano_width, vp, color)
	boostio.drawRectangle(5, y + 1, piano_width - 10, vp.piano_key_height - 2, color.r, color.g, color.b, 1.0)
end

local function draw_piano_key_rect_with_alpha(y, piano_width, vp, color, alpha)
	boostio.drawRectangle(5, y + 1, piano_width - 10, vp.piano_key_height - 2, color.r, color.g, color.b, alpha)
end

local function draw_piano_key_separator(y, piano_width, theme)
	local sep_color = boostio.hexToRgb(theme.piano_key_separator)
	boostio.drawLine(0, y, piano_width, y, sep_color.r, sep_color.g, sep_color.b, 1.0)
end

local function draw_piano_key_text(key, y, vp, theme, utils)
	if vp.piano_key_height < 15.0 then
		return
	end

	local note_name = utils.get_note_name(key)
	local text_color_key = utils.is_black_key(key) and "piano_key_black_text" or "piano_key_white_text"
	local text_color = boostio.hexToRgb(theme[text_color_key])
	boostio.drawText(note_name, 10, y + vp.piano_key_height / 2.0 + 4.0, 12, text_color.r, text_color.g, text_color.b, 1.0)
end

local function draw_folded_piano_key(row_y, key, piano_width, vp, state, theme, utils)
	if state.show_scale_highlights then
		local is_root = boostio.isRootNote(key, state.selected_root)
		local color_key = is_root and "scale_root_piano" or "scale_note_piano"
		local color = boostio.hexToRgb(theme[color_key])
		draw_piano_key_rect(row_y, piano_width, vp, color)
	else
		local color = boostio.hexToRgb(theme.piano_key_white)
		draw_piano_key_rect(row_y, piano_width, vp, color)
	end

	draw_piano_key_separator(row_y, piano_width, theme)
	draw_piano_key_text(key, row_y, vp, theme, utils)
end

local function draw_folded_piano_keys(vp, state, theme, options, utils)
	if not boostio or not boostio.isNoteInScale then
		return
	end

	local piano_width = vp.grid_x
	local row_y = vp.grid_y
	local current_row = 0

	for key = options.piano_key_max, options.piano_key_min, -1 do
		local success, in_scale = pcall(boostio.isNoteInScale, key, state.selected_scale, state.selected_root)
		if success and in_scale then
			if current_row >= vp.note_offset then
				if row_y >= vp.grid_y + vp.grid_height then
					break
				end

				draw_folded_piano_key(row_y, key, piano_width, vp, state, theme, utils)
				row_y = row_y + vp.piano_key_height
			end
			current_row = current_row + 1
		end
	end
end

local function draw_scale_overlay(y, piano_width, vp, key, state, theme)
	if not state.show_scale_highlights then
		return
	end

	if boostio.isRootNote(key, state.selected_root) then
		local color = boostio.hexToRgb(theme.scale_root_piano)
		draw_piano_key_rect_with_alpha(y, piano_width, vp, color, 0.7)
		return
	end

	if not boostio or not boostio.isNoteInScale then
		return
	end

	local success, in_scale = pcall(boostio.isNoteInScale, key, state.selected_scale, state.selected_root)
	if success and in_scale then
		local color = boostio.hexToRgb(theme.scale_note_piano)
		draw_piano_key_rect_with_alpha(y, piano_width, vp, color, 0.55)
	end
end

local function draw_unfolded_piano_key(y, key, piano_width, vp, state, theme, utils)
	local is_black = utils.is_black_key(key)
	local color_key = is_black and "piano_key_black" or "piano_key_white"
	local color = boostio.hexToRgb(theme[color_key])
	draw_piano_key_rect(y, piano_width, vp, color)

	draw_scale_overlay(y, piano_width, vp, key, state, theme)
	draw_piano_key_separator(y, piano_width, theme)

	if not is_black then
		draw_piano_key_text(key, y, vp, theme, utils)
	end
end

local function draw_unfolded_piano_keys(vp, state, theme, options, utils)
	local piano_width = vp.grid_x
	local start_key, end_key = get_visible_key_range(vp, options)

	for key = start_key, end_key do
		local y = boostio.pianoKeyToY(key)
		draw_unfolded_piano_key(y, key, piano_width, vp, state, theme, utils)
	end
end

function rendering.render_piano_keys(state, theme, options, utils)
	local vp = state.viewport
	local piano_width = vp.grid_x

	draw_piano_key_background(vp.grid_y, piano_width, vp, theme)

	if state.fold_mode then
		draw_folded_piano_keys(vp, state, theme, options, utils)
		return
	end

	draw_unfolded_piano_keys(vp, state, theme, options, utils)
end

local function check_if_has_solo(state)
	for i = 1, 8 do
		if state.voice_solo[i] then
			return true
		end
	end
	return false
end

local function is_note_audible(voice, state, has_solo)
	if has_solo then
		return state.voice_solo[voice + 1]
	end
	return not state.voice_muted[voice + 1]
end

local function calculate_note_color(voice_color, is_audible, is_selected)
	local note_color = { r = voice_color.r, g = voice_color.g, b = voice_color.b, a = 1.0 }

	if not is_audible then
		note_color.r = note_color.r * 0.3
		note_color.g = note_color.g * 0.3
		note_color.b = note_color.b * 0.3
		note_color.a = 0.5
		return note_color
	end

	if is_selected then
		note_color.r = math.min(1.0, note_color.r * 1.3)
		note_color.g = math.min(1.0, note_color.g * 1.3)
		note_color.b = math.min(1.0, note_color.b * 1.3)
	end

	return note_color
end

local function is_note_visible(rect, vp)
	if rect.x + rect.width < vp.grid_x or rect.x > vp.grid_x + vp.grid_width then
		return false
	end
	if rect.y + rect.height < vp.grid_y or rect.y > vp.grid_y + vp.grid_height then
		return false
	end
	return true
end

local function draw_note(rect, note_color, is_selected)
	boostio.drawRoundedRectangle(rect.x, rect.y, rect.width, rect.height, 3, note_color.r, note_color.g, note_color.b, note_color.a)

	if is_selected then
		boostio.strokeRoundedRectangle(rect.x, rect.y, rect.width, rect.height, 3, 1.0, 1.0, 1.0, 1.0)
	end
end

function rendering.render_notes(state, theme, options, utils)
	local vp = state.viewport
	local has_solo = check_if_has_solo(state)

	for _, note in ipairs(state.notes) do
		local voice = note.voice % 8

		if not state.voice_hidden[voice + 1] then
			local is_audible = is_note_audible(voice, state, has_solo)
			local rect = utils.get_note_rect(vp, note, state.fold_mode, state.selected_scale, state.selected_root, options)

			if is_note_visible(rect, vp) then
				local voice_color = boostio.hexToRgb(theme.voice_colors[voice + 1])
				local is_selected = boostio.isNoteSelected(note.id)
				local note_color = calculate_note_color(voice_color, is_audible, is_selected)

				draw_note(rect, note_color, is_selected)
			end
		end
	end
end

function rendering.render_selection_box(mouse_state)
	if not mouse_state.selection_box.active then
		return
	end

	local box = mouse_state.selection_box
	local min_x = math.min(box.start_x, box.end_x)
	local max_x = math.max(box.start_x, box.end_x)
	local min_y = math.min(box.start_y, box.end_y)
	local max_y = math.max(box.start_y, box.end_y)
	local width = max_x - min_x
	local height = max_y - min_y

	boostio.drawRectangle(min_x, min_y, width, height, 0.5, 0.7, 1.0, 0.2)
	boostio.strokeRectangle(min_x, min_y, width, height, 0.5, 0.7, 1.0, 0.8)
end

function rendering.render_measure_gutter(state, theme)
	local vp = state.viewport
	local gutter_height = 10
	local gutter_y = vp.grid_y - gutter_height

	local bg = boostio.hexToRgb(theme.background)
	boostio.drawRectangle(vp.grid_x, gutter_y, vp.grid_width, gutter_height, bg.r, bg.g, bg.b, 1.0)

	local ms_per_beat = 60000.0 / state.bpm
	local ms_per_bar = ms_per_beat * 4.0

	local start_ms = vp.time_offset
	local end_ms = start_ms + (vp.grid_width / vp.pixels_per_ms)

	local first_bar = math.floor(start_ms / ms_per_bar)
	local last_bar = math.floor(end_ms / ms_per_bar) + 1

	local text_color = boostio.hexToRgb(theme.grid_beat_line)

	for bar = first_bar, last_bar do
		local bar_ms_float = bar * ms_per_bar
		local x = boostio.msToX(bar_ms_float)

		if x >= vp.grid_x and x <= vp.grid_x + vp.grid_width then
			local measure_number = bar + 1
			boostio.drawText(
				tostring(measure_number),
				x + 4,
				gutter_y + gutter_height / 2 + 5,
				12,
				text_color.r,
				text_color.g,
				text_color.b,
				0.8
			)
		end
	end
end

function rendering.render_playhead(state, theme)
	local vp = state.viewport
	local x = boostio.msToX(state.playhead_ms)

	if x < vp.grid_x or x > vp.grid_x + vp.grid_width then
		return
	end

	local color = boostio.hexToRgb(theme.playhead)
	local win_width, win_height = boostio.getWindowSize()
	boostio.drawRectangle(x - 1, vp.grid_y, 1, win_height, color.r, color.g, color.b, theme.playhead_alpha)
	boostio.drawRectangle(x - 5, vp.grid_y - 10, 10, 10, color.r, color.g, color.b, theme.playhead_alpha)
end

return rendering
