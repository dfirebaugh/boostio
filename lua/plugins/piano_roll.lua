local piano_roll = {}

piano_roll.enabled = true

local function is_voice_visible(voice)
	if voice_controls and voice_controls.isVoiceVisible then
		return voice_controls.isVoiceVisible(voice)
	end
	return true
end

local default_options = {
	edge_threshold = 5,
	drag_threshold = 5,
	double_click_threshold_ms = 500,
	min_note_duration_ms = 10,
	piano_key_min = 45,
	piano_key_max = 99,
}

local options = {}

function piano_roll.init(config_options)
	for k, v in pairs(default_options) do
		options[k] = v
	end

	if config_options then
		for k, v in pairs(config_options) do
			if default_options[k] ~= nil then
				options[k] = v
			end
		end
	end
end

local mouse_state = {
	down = false,
	down_x = 0,
	down_y = 0,
	drag_mode = "none",
	drag_started = false,
	selection_box = {
		active = false,
		start_x = 0,
		start_y = 0,
		end_x = 0,
		end_y = 0,
	},
	drag_data = {
		initial_positions = {},
		initial_durations = {},
		initial_piano_keys = {},
		primary_note_id = nil,
		copied_note_ids = {},
		original_note_ids = {},
	},
	clicked_note_id = nil,
	resize_from_left = false,
	last_click_time = 0,
	last_clicked_note_id = nil,
	last_preview_piano_key = nil,
}

local function is_black_key(piano_key)
	local note = piano_key % 12
	return (note == 1 or note == 3 or note == 6 or note == 8 or note == 10)
end

local function get_note_name(piano_key)
	local names = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }
	local note = piano_key % 12
	local octave = math.floor(piano_key / 12) - 1
	return names[note + 1] .. octave
end

local function piano_key_to_folded_row(piano_key, scale, root)
	if not scale or not root or not boostio or not boostio.isNoteInScale then
		return -1
	end

	local row = 0
	for key = options.piano_key_max, options.piano_key_min, -1 do
		local success, in_scale = pcall(boostio.isNoteInScale, key, scale, root)
		if success and in_scale then
			if key == piano_key then
				return row
			end
			row = row + 1
		end
	end
	return -1
end

local function y_to_piano_key_folded(y, vp, scale, root)
	if not scale or not root or not boostio or not boostio.isNoteInScale then
		return 72
	end

	local relative_y = y - vp.grid_y
	local target_row = math.floor(relative_y / vp.piano_key_height) + vp.note_offset

	local current_row = 0
	for key = options.piano_key_max, options.piano_key_min, -1 do
		local success, in_scale = pcall(boostio.isNoteInScale, key, scale, root)
		if success and in_scale then
			if current_row == target_row then
				return key
			end
			current_row = current_row + 1
		end
	end

	return 72
end

local function get_note_rect(vp, note, fold_mode, scale, root)
	local x = boostio.msToX(note.ms)
	local y

	if fold_mode then
		local folded_row = piano_key_to_folded_row(note.piano_key, scale, root)
		if folded_row < 0 then
			return { x = -1000, y = -1000, width = 0, height = 0 }
		end
		local offset_row = folded_row - vp.note_offset
		y = vp.grid_y + offset_row * vp.piano_key_height
	else
		y = boostio.pianoKeyToY(note.piano_key)
	end

	local width = note.duration_ms * vp.pixels_per_ms
	local height = vp.piano_key_height - 2

	return { x = x, y = y, width = width, height = height }
end

local function point_in_rect(px, py, rect)
	return px >= rect.x and px <= rect.x + rect.width and py >= rect.y and py <= rect.y + rect.height
end

local function find_note_by_id(state, note_id)
	for _, note in ipairs(state.notes) do
		if note.id == note_id then
			return note
		end
	end
	return nil
end

local function store_note_drag_data(note_id, note, drag_data, store_position)
	if store_position then
		drag_data.initial_positions[note_id] = note.ms
	end
	drag_data.initial_durations[note_id] = note.duration_ms
	drag_data.initial_piano_keys[note_id] = note.piano_key
end

local function for_each_selected_note(state, callback)
	local selection = boostio.getSelection()
	for _, selected_id in ipairs(selection) do
		local note = find_note_by_id(state, selected_id)
		if note then
			callback(selected_id, note)
		end
	end
end

local function find_note_at_position(x, y, state)
	for i = #state.notes, 1, -1 do
		local note = state.notes[i]
		local rect = get_note_rect(state.viewport, note, state.fold_mode, state.selected_scale,
			state.selected_root)

		if point_in_rect(x, y, rect) then
			local is_left_edge = (x - rect.x) <= options.edge_threshold
			local is_right_edge = (rect.x + rect.width - x) <= options.edge_threshold

			return note.id, is_left_edge, is_right_edge
		end
	end
	return nil, false, false
end

local function snap_to_grid(ms, state)
	if not state.snap_enabled then
		return ms
	end

	local ms_per_beat = 60000.0 / state.bpm
	local ms_per_32nd = ms_per_beat / 8.0
	local grid_index = math.floor(ms / ms_per_32nd + 0.5)
	return grid_index * ms_per_32nd
end

local function handle_mouse_down(x, y, button, state)
	if button ~= boostio.MOUSE_BUTTON_LEFT then
		return
	end

	local vp = state.viewport

	if x >= 0 and x < vp.grid_x and y >= vp.grid_y and y < vp.grid_y + vp.grid_height then
		local piano_key
		if state.fold_mode then
			piano_key = y_to_piano_key_folded(y, vp, state.selected_scale, state.selected_root)
		else
			piano_key = boostio.yToPianoKey(y)
		end
		boostio.playPreviewNote(piano_key)
		return
	end

	if x < vp.grid_x or x > vp.grid_x + vp.grid_width or y < vp.grid_y or y > vp.grid_y + vp.grid_height then
		return
	end

	mouse_state.down = true
	mouse_state.down_x = x
	mouse_state.down_y = y
	mouse_state.drag_started = false

	local note_id, is_left_edge, is_right_edge = find_note_at_position(x, y, state)
	mouse_state.clicked_note_id = note_id

	local ctrl_held = boostio.isKeyDown("ctrl")
	local shift_held = boostio.isKeyDown("shift")

	if note_id then
		local note = find_note_by_id(state, note_id)
		if not note or not is_voice_visible(note.voice) then
			return
		end

		if is_left_edge then
			mouse_state.drag_mode = "resize_left"
			mouse_state.resize_from_left = true

			mouse_state.drag_data.initial_positions = {}
			mouse_state.drag_data.initial_durations = {}
			mouse_state.drag_data.initial_piano_keys = {}

			local selection = boostio.getSelection()
			if #selection == 0 or not boostio.isNoteSelected(note_id) then
				if note then
					store_note_drag_data(note_id, note, mouse_state.drag_data, true)
				end
			else
				for_each_selected_note(state, function(selected_id, note)
					store_note_drag_data(selected_id, note, mouse_state.drag_data, true)
				end)
			end
		elseif is_right_edge then
			mouse_state.drag_mode = "resize_right"
			mouse_state.resize_from_left = false

			mouse_state.drag_data.initial_positions = {}
			mouse_state.drag_data.initial_durations = {}
			mouse_state.drag_data.initial_piano_keys = {}

			local selection = boostio.getSelection()
			if #selection == 0 or not boostio.isNoteSelected(note_id) then
				if note then
					store_note_drag_data(note_id, note, mouse_state.drag_data, false)
				end
			else
				for_each_selected_note(state, function(selected_id, note)
					store_note_drag_data(selected_id, note, mouse_state.drag_data, false)
				end)
			end
		else
			if shift_held then
				mouse_state.drag_mode = "copy"
			else
				mouse_state.drag_mode = "move"
			end

			mouse_state.drag_data.primary_note_id = note_id
			mouse_state.drag_data.initial_positions = {}
			mouse_state.drag_data.initial_durations = {}
			mouse_state.drag_data.initial_piano_keys = {}
			mouse_state.drag_data.copied_note_ids = {}
			mouse_state.drag_data.original_note_ids = {}

			local selection = boostio.getSelection()
			if #selection == 0 or not boostio.isNoteSelected(note_id) then
				local note = find_note_by_id(state, note_id)
				if note and is_voice_visible(note.voice) then
					if not ctrl_held and not shift_held then
						boostio.clearSelection()
					end
					boostio.selectNote(note_id)
					store_note_drag_data(note_id, note, mouse_state.drag_data, true)
				end
			else
				if ctrl_held then
					boostio.deselectNote(note_id)
				else
					for_each_selected_note(state, function(selected_id, note)
						store_note_drag_data(selected_id, note, mouse_state.drag_data, true)
					end)
				end
			end
		end
	else
		if not ctrl_held and not shift_held then
			boostio.clearSelection()
		end
		mouse_state.drag_mode = "box_select"
		mouse_state.selection_box.active = true
		mouse_state.selection_box.start_x = x
		mouse_state.selection_box.start_y = y
		mouse_state.selection_box.end_x = x
		mouse_state.selection_box.end_y = y
	end
end

local function handle_mouse_move(x, y, state)
	if not mouse_state.down then
		return
	end

	local dx = x - mouse_state.down_x
	local dy = y - mouse_state.down_y
	local distance = math.sqrt(dx * dx + dy * dy)

	if distance > options.drag_threshold then
		if not mouse_state.drag_started then
			mouse_state.drag_started = true

			if mouse_state.drag_mode == "copy" then
				local new_positions = {}
				local new_durations = {}
				local new_piano_keys = {}
				local copied_ids = {}
				local original_ids = {}

				for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
					local initial_duration = mouse_state.drag_data.initial_durations[note_id]
					local initial_piano_key = mouse_state.drag_data.initial_piano_keys[note_id]

					for _, note in ipairs(state.notes) do
						if note.id == note_id then
							local params = {
								voice = note.voice,
								waveform = note.waveform,
								duty_cycle = note.duty_cycle,
								decay = note.decay,
								amplitude_dbfs = note.amplitude_dbfs,
								nes_noise_period = note.nes_noise_period,
								nes_noise_mode_flag = note.nes_noise_mode_flag,
							}
							local new_note_id = boostio.addNote(note.ms, note.piano_key,
								note.duration_ms, params)
							if new_note_id then
								table.insert(copied_ids, new_note_id)
								table.insert(original_ids, note_id)
								new_positions[new_note_id] = initial_ms
								new_durations[new_note_id] = initial_duration
								new_piano_keys[new_note_id] = initial_piano_key
							end
							break
						end
					end
				end

				mouse_state.drag_data.initial_positions = new_positions
				mouse_state.drag_data.initial_durations = new_durations
				mouse_state.drag_data.initial_piano_keys = new_piano_keys
				mouse_state.drag_data.copied_note_ids = copied_ids
				mouse_state.drag_data.original_note_ids = original_ids

				if #copied_ids > 0 then
					mouse_state.drag_data.primary_note_id = copied_ids[1]
				end

				boostio.clearSelection()
				for _, copied_id in ipairs(copied_ids) do
					boostio.selectNote(copied_id)
				end
			end
		end
	end

	if not mouse_state.drag_started then
		return
	end

	if mouse_state.drag_mode == "move" or mouse_state.drag_mode == "copy" then
		local delta_ms = boostio.xToMs(x) - boostio.xToMs(mouse_state.down_x)
		local delta_piano_key

		if state.fold_mode then
			local current_key = y_to_piano_key_folded(y, state.viewport, state.selected_scale,
				state.selected_root)
			local initial_key =
			    y_to_piano_key_folded(mouse_state.down_y, state.viewport, state.selected_scale,
				    state.selected_root)
			local current_row = piano_key_to_folded_row(current_key, state.selected_scale,
				state.selected_root)
			local initial_row = piano_key_to_folded_row(initial_key, state.selected_scale,
				state.selected_root)
			delta_piano_key = current_row - initial_row
		else
			delta_piano_key = boostio.yToPianoKey(y) - boostio.yToPianoKey(mouse_state.down_y)
		end

		local primary_note = nil
		if mouse_state.drag_data.primary_note_id then
			primary_note = find_note_by_id(state, mouse_state.drag_data.primary_note_id)
		end

		if primary_note and state.snap_enabled then
			local initial_ms = mouse_state.drag_data.initial_positions[primary_note.id]
			local target_ms = initial_ms + delta_ms
			local snapped_ms = snap_to_grid(target_ms, state)
			delta_ms = snapped_ms - initial_ms
		end

		for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
			local initial_piano_key = mouse_state.drag_data.initial_piano_keys[note_id]
			local note = find_note_by_id(state, note_id)
			if note then
				note.ms = math.max(0, initial_ms + delta_ms)

				if state.fold_mode then
					local initial_row =
					    piano_key_to_folded_row(initial_piano_key, state.selected_scale,
						    state.selected_root)
					local target_row = initial_row + delta_piano_key

					local current_row = 0
					local new_key = initial_piano_key
					if boostio and boostio.isNoteInScale then
						for key = options.piano_key_max, options.piano_key_min, -1 do
							local success, in_scale =
							    pcall(boostio.isNoteInScale, key, state.selected_scale,
								    state.selected_root)
							if success and in_scale then
								if current_row == target_row then
									new_key = key
									break
								end
								current_row = current_row + 1
							end
						end
					end
					note.piano_key = new_key
				else
					local new_key = initial_piano_key + delta_piano_key
					note.piano_key = math.max(0, math.min(127, math.floor(new_key + 0.5)))
				end

				if note_id == mouse_state.drag_data.primary_note_id and playback_controls and
				    playback_controls.is_preview_on_drag_enabled and
				    playback_controls.is_preview_on_drag_enabled() and not state.is_playing then
					if note.piano_key ~= mouse_state.last_preview_piano_key then
						boostio.playPreviewNote(note.piano_key)
						mouse_state.last_preview_piano_key = note.piano_key
					end
				end
			end
		end
	elseif mouse_state.drag_mode == "resize_left" then
		local delta_ms = boostio.xToMs(x) - boostio.xToMs(mouse_state.down_x)

		for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
			local initial_duration = mouse_state.drag_data.initial_durations[note_id]
			local target_ms = initial_ms + delta_ms

			if state.snap_enabled then
				target_ms = snap_to_grid(target_ms, state)
				delta_ms = target_ms - initial_ms
			end

			local new_duration = initial_duration - delta_ms
			if new_duration < options.min_note_duration_ms then
				delta_ms = initial_duration - options.min_note_duration_ms
				new_duration = options.min_note_duration_ms
			end

			local note = find_note_by_id(state, note_id)
			if note then
				note.ms = math.max(0, initial_ms + delta_ms)
				note.duration_ms = math.floor(new_duration)
			end
		end
	elseif mouse_state.drag_mode == "resize_right" then
		local delta_ms = boostio.xToMs(x) - boostio.xToMs(mouse_state.down_x)

		for note_id, initial_duration in pairs(mouse_state.drag_data.initial_durations) do
			local new_duration = initial_duration + delta_ms

			if state.snap_enabled then
				local note = find_note_by_id(state, note_id)
				if note then
					local end_ms = note.ms + initial_duration + delta_ms
					local snapped_end_ms = snap_to_grid(end_ms, state)
					new_duration = snapped_end_ms - note.ms
				end
			end

			if new_duration < options.min_note_duration_ms then
				new_duration = options.min_note_duration_ms
			end

			local note = find_note_by_id(state, note_id)
			if note then
				note.duration_ms = math.floor(new_duration)
			end
		end
	elseif mouse_state.drag_mode == "box_select" then
		mouse_state.selection_box.end_x = x
		mouse_state.selection_box.end_y = y

		local box = mouse_state.selection_box
		local min_x = math.min(box.start_x, box.end_x)
		local max_x = math.max(box.start_x, box.end_x)
		local min_y = math.min(box.start_y, box.end_y)
		local max_y = math.max(box.start_y, box.end_y)

		local ctrl_held = boostio.isKeyDown("ctrl")
		local shift_held = boostio.isKeyDown("shift")
		if not ctrl_held and not shift_held then
			boostio.clearSelection()
		end

		for _, note in ipairs(state.notes) do
			local rect = get_note_rect(state.viewport, note, state.fold_mode, state.selected_scale,
				state.selected_root)

			local note_min_x = rect.x
			local note_max_x = rect.x + rect.width
			local note_min_y = rect.y
			local note_max_y = rect.y + rect.height

			local intersects = not (
				note_max_x < min_x
				or note_min_x > max_x
				or note_max_y < min_y
				or note_min_y > max_y
			)

			if intersects and is_voice_visible(note.voice) then
				boostio.selectNote(note.id)
			end
		end
	end
end

local function handle_mouse_up(x, y, button, state)
	if button ~= boostio.MOUSE_BUTTON_LEFT then
		return
	end

	if mouse_state.drag_started then
		if mouse_state.drag_mode == "move" or mouse_state.drag_mode == "copy" then
			for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
				local initial_piano_key = mouse_state.drag_data.initial_piano_keys[note_id]
				local note = find_note_by_id(state, note_id)
				if note then
					local delta_ms = math.floor(note.ms - initial_ms)
					local delta_piano_key = math.floor(note.piano_key - initial_piano_key)
					boostio.moveNote(note_id, delta_ms, delta_piano_key)
				end
			end
		elseif mouse_state.drag_mode == "resize_left" then
			for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
				local initial_duration = mouse_state.drag_data.initial_durations[note_id]
				local note = find_note_by_id(state, note_id)
				if note then
					local delta_ms = math.floor(note.ms - initial_ms)
					local delta_duration = math.floor(note.duration_ms - initial_duration)
					boostio.resizeNote(note_id, delta_duration, true)
				end
			end
		elseif mouse_state.drag_mode == "resize_right" then
			for note_id, initial_duration in pairs(mouse_state.drag_data.initial_durations) do
				local note = find_note_by_id(state, note_id)
				if note then
					local delta_duration = math.floor(note.duration_ms - initial_duration)
					boostio.resizeNote(note_id, delta_duration, false)
				end
			end
		end
	end

	if mouse_state.down and not mouse_state.drag_started then
		local note_id = mouse_state.clicked_note_id
		local ctrl_held = boostio.isKeyDown("ctrl")
		local shift_held = boostio.isKeyDown("shift")
		local current_time = os.clock() * 1000

		if note_id then
			local time_since_last_click = current_time - mouse_state.last_click_time
			local is_double_click = (time_since_last_click < options.double_click_threshold_ms)
			    and (note_id == mouse_state.last_clicked_note_id)

			if is_double_click then
				boostio.deleteNote(note_id)
				mouse_state.last_click_time = 0
				mouse_state.last_clicked_note_id = nil
			else
				mouse_state.last_click_time = current_time
				mouse_state.last_clicked_note_id = note_id

				local note = find_note_by_id(state, note_id)
				if note and is_voice_visible(note.voice) then
					if playback_controls and playback_controls.is_preview_on_drag_enabled and
					    playback_controls.is_preview_on_drag_enabled() and not state.is_playing then
						boostio.playPreviewNote(note.piano_key)
					end

					if ctrl_held then
						if boostio.isNoteSelected(note_id) then
							boostio.deselectNote(note_id)
						else
							boostio.selectNote(note_id)
						end
					elseif shift_held then
						boostio.selectNote(note_id)
					else
						boostio.clearSelection()
						boostio.selectNote(note_id)
					end
				end
			end
		else
			mouse_state.last_click_time = 0
			mouse_state.last_clicked_note_id = nil

			if not ctrl_held and not shift_held then
				boostio.clearSelection()
			end

			local vp = state.viewport
			if
			    x >= vp.grid_x
			    and x <= vp.grid_x + vp.grid_width
			    and y >= vp.grid_y
			    and y <= vp.grid_y + vp.grid_height
			then
				local ms = boostio.xToMs(x)
				local piano_key
				if state.fold_mode then
					piano_key = y_to_piano_key_folded(y, vp, state.selected_scale,
						state.selected_root)
				else
					piano_key = boostio.yToPianoKey(y)
				end

				if state.snap_enabled and state.snap_ms > 0 then
					ms = math.floor(ms / state.snap_ms + 0.5) * state.snap_ms
				end

				ms = math.max(0, ms)

				if piano_key >= 0 and piano_key <= 87 then
					local ms_per_beat = 60000.0 / state.bpm
					local default_duration = ms_per_beat

					local new_note_id = boostio.addNote(ms, piano_key, default_duration)
					if new_note_id then
						local new_note = find_note_by_id(state, new_note_id)
						if new_note and is_voice_visible(new_note.voice) then
							boostio.clearSelection()
							boostio.selectNote(new_note_id)
						end
					end
				end
			end
		end
	end

	mouse_state.down = false
	mouse_state.drag_mode = "none"
	mouse_state.drag_started = false
	mouse_state.selection_box.active = false
	mouse_state.clicked_note_id = nil
	mouse_state.last_preview_piano_key = nil
	mouse_state.drag_data.initial_positions = {}
	mouse_state.drag_data.initial_durations = {}
	mouse_state.drag_data.initial_piano_keys = {}
	mouse_state.drag_data.primary_note_id = nil
	mouse_state.drag_data.copied_note_ids = {}
	mouse_state.drag_data.original_note_ids = {}
end

local function update(state)
	local mouse_x, mouse_y = boostio.getMousePosition()

	if boostio.isMouseButtonPressed(boostio.MOUSE_BUTTON_LEFT) then
		handle_mouse_down(mouse_x, mouse_y, boostio.MOUSE_BUTTON_LEFT, state)
	end

	if mouse_state.down then
		handle_mouse_move(mouse_x, mouse_y, state)
	end

	if not boostio.isMouseButtonDown(boostio.MOUSE_BUTTON_LEFT) and mouse_state.down then
		handle_mouse_up(mouse_x, mouse_y, boostio.MOUSE_BUTTON_LEFT, state)
	end
end

local function render_grid(state, theme)
	local vp = state.viewport

	local bg = boostio.hexToRgb(theme.grid_background)
	boostio.drawRectangle(vp.grid_x, vp.grid_y, vp.grid_width, vp.grid_height, bg.r, bg.g, bg.b, 1.0)

	local visible_keys = math.floor(vp.grid_height / vp.piano_key_height) + 2
	local start_key = options.piano_key_max - (vp.note_offset + visible_keys)
	local end_key = options.piano_key_max - vp.note_offset

	if start_key < options.piano_key_min then
		start_key = options.piano_key_min
	end
	if end_key > options.piano_key_max then
		end_key = options.piano_key_max
	end

	if state.fold_mode then
		local row_y = vp.grid_y
		local current_row = 0

		if boostio and boostio.isNoteInScale then
			for key = options.piano_key_max, options.piano_key_min, -1 do
				local success, in_scale = pcall(boostio.isNoteInScale, key, state.selected_scale,
					state.selected_root)
				if success and in_scale then
					if current_row >= vp.note_offset then
						if row_y >= vp.grid_y + vp.grid_height then
							break
						end

						if state.show_scale_highlights then
							if boostio.isRootNote(key, state.selected_root) then
								local color = boostio.hexToRgb(theme.scale_root_grid)
								boostio.drawRectangle(
									vp.grid_x,
									row_y,
									vp.grid_width,
									vp.piano_key_height,
									color.r,
									color.g,
									color.b,
									theme.scale_root_grid_alpha
								)
							else
								local color = boostio.hexToRgb(theme.scale_note_grid)
								boostio.drawRectangle(
									vp.grid_x,
									row_y,
									vp.grid_width,
									vp.piano_key_height,
									color.r,
									color.g,
									color.b,
									theme.scale_note_grid_alpha
								)
							end
						end

						local line_color = boostio.hexToRgb(theme.grid_line)
						boostio.drawLine(
							vp.grid_x,
							row_y,
							vp.grid_x + vp.grid_width,
							row_y,
							line_color.r,
							line_color.g,
							line_color.b,
							1.0
						)

						row_y = row_y + vp.piano_key_height
					end
					current_row = current_row + 1
				end
			end
		end
	else
		for key = start_key, end_key do
			local y = boostio.pianoKeyToY(key)

			if is_black_key(key) then
				local color = boostio.hexToRgb(theme.piano_key_black)
				boostio.drawRectangle(vp.grid_x, y, vp.grid_width, vp.piano_key_height, color.r, color.g,
					color.b, 1.0)
			end

			if state.show_scale_highlights then
				if boostio.isRootNote(key, state.selected_root) then
					local color = boostio.hexToRgb(theme.scale_root_grid)
					boostio.drawRectangle(
						vp.grid_x,
						y,
						vp.grid_width,
						vp.piano_key_height,
						color.r,
						color.g,
						color.b,
						theme.scale_root_grid_alpha
					)
				else
					local in_scale = false
					if boostio and boostio.isNoteInScale then
						local success, result =
						    pcall(boostio.isNoteInScale, key, state.selected_scale,
							    state.selected_root)
						in_scale = success and result
					end
					if in_scale then
						local color = boostio.hexToRgb(theme.scale_note_grid)
						boostio.drawRectangle(
							vp.grid_x,
							y,
							vp.grid_width,
							vp.piano_key_height,
							color.r,
							color.g,
							color.b,
							theme.scale_note_grid_alpha
						)
					end
				end
			end

			local line_color = boostio.hexToRgb(theme.grid_line)
			boostio.drawLine(vp.grid_x, y, vp.grid_x + vp.grid_width, y, line_color.r, line_color.g,
				line_color.b, 1.0)
		end
	end

	local ms_per_beat = 60000.0 / state.bpm
	local ms_per_bar = ms_per_beat * 4.0

	local start_ms = vp.time_offset
	local end_ms = start_ms + (vp.grid_width / vp.pixels_per_ms)

	if state.snap_enabled then
		local ms_per_32nd = ms_per_beat / 8.0
		local first_32nd = math.floor(start_ms / ms_per_32nd)
		local last_32nd = math.floor(end_ms / ms_per_32nd) + 1

		for note_32nd = first_32nd, last_32nd do
			local note_ms_float = note_32nd * ms_per_32nd
			local x = boostio.msToX(note_ms_float)

			if x >= vp.grid_x and x <= vp.grid_x + vp.grid_width then
				local is_beat = (note_32nd % 8) == 0
				local is_bar = is_beat and ((math.floor(note_32nd / 8) % 4) == 0)

				if is_bar then
					local bar = math.floor(note_32nd / 32)
					if bar % 2 == 0 then
						local color = boostio.hexToRgb(theme.grid_line)
						boostio.drawRectangle(
							x,
							vp.grid_y,
							ms_per_bar * vp.pixels_per_ms,
							vp.grid_height,
							color.r,
							color.g,
							color.b,
							0.5
						)
					end

					local beat_color = boostio.hexToRgb(theme.grid_beat_line)
					boostio.drawLine(
						x,
						vp.grid_y,
						x,
						vp.grid_y + vp.grid_height,
						beat_color.r,
						beat_color.g,
						beat_color.b,
						1.0
					)
				elseif is_beat then
					local color = boostio.hexToRgb(theme.grid_line)
					boostio.drawLine(x, vp.grid_y, x, vp.grid_y + vp.grid_height, color.r, color.g,
						color.b, 0.78)
				else
					local color = boostio.hexToRgb(theme.grid_line)
					boostio.drawLine(x, vp.grid_y, x, vp.grid_y + vp.grid_height, color.r, color.g,
						color.b, 0.23)
				end
			end
		end
	else
		local first_bar = math.floor(start_ms / ms_per_bar)
		local last_bar = math.floor(end_ms / ms_per_bar) + 1

		for bar = first_bar, last_bar do
			local bar_ms_float = bar * ms_per_bar
			local x = boostio.msToX(bar_ms_float)

			if x >= vp.grid_x and x <= vp.grid_x + vp.grid_width then
				if bar % 2 == 0 then
					local color = boostio.hexToRgb(theme.grid_line)
					boostio.drawRectangle(
						x,
						vp.grid_y,
						ms_per_bar * vp.pixels_per_ms,
						vp.grid_height,
						color.r,
						color.g,
						color.b,
						0.5
					)
				end

				local beat_color = boostio.hexToRgb(theme.grid_beat_line)
				boostio.drawLine(
					x,
					vp.grid_y,
					x,
					vp.grid_y + vp.grid_height,
					beat_color.r,
					beat_color.g,
					beat_color.b,
					1.0
				)
			end
		end
	end
end

local function render_piano_keys(state, theme)
	local vp = state.viewport
	local piano_width = vp.grid_x

	local bg = boostio.hexToRgb(theme.background)
	boostio.drawRectangle(0, vp.grid_y, piano_width, vp.grid_height, bg.r, bg.g, bg.b, 1.0)

	local visible_keys = math.floor(vp.grid_height / vp.piano_key_height) + 2
	local start_key = options.piano_key_max - (vp.note_offset + visible_keys)
	local end_key = options.piano_key_max - vp.note_offset

	if start_key < options.piano_key_min then
		start_key = options.piano_key_min
	end
	if end_key > options.piano_key_max then
		end_key = options.piano_key_max
	end

	if state.fold_mode then
		local row_y = vp.grid_y
		local current_row = 0

		if boostio and boostio.isNoteInScale then
			for key = options.piano_key_max, options.piano_key_min, -1 do
				local success, in_scale = pcall(boostio.isNoteInScale, key, state.selected_scale,
					state.selected_root)
				if success and in_scale then
					if current_row >= vp.note_offset then
						if row_y >= vp.grid_y + vp.grid_height then
							break
						end

						if state.show_scale_highlights then
							local is_root = boostio.isRootNote(key, state.selected_root)

							if is_root then
								local color = boostio.hexToRgb(theme.scale_root_piano)
								boostio.drawRectangle(
									5,
									row_y + 1,
									piano_width - 10,
									vp.piano_key_height - 2,
									color.r,
									color.g,
									color.b,
									1.0
								)
							else
								local color = boostio.hexToRgb(theme.scale_note_piano)
								boostio.drawRectangle(
									5,
									row_y + 1,
									piano_width - 10,
									vp.piano_key_height - 2,
									color.r,
									color.g,
									color.b,
									1.0
								)
							end
						else
							local color = boostio.hexToRgb(theme.piano_key_white)
							boostio.drawRectangle(
								5,
								row_y + 1,
								piano_width - 10,
								vp.piano_key_height - 2,
								color.r,
								color.g,
								color.b,
								1.0
							)
						end

						local sep_color = boostio.hexToRgb(theme.piano_key_separator)
						boostio.drawLine(0, row_y, piano_width, row_y, sep_color.r, sep_color.g,
							sep_color.b, 1.0)

						if vp.piano_key_height >= 15.0 then
							local note_name = get_note_name(key)
							local text_color_key = is_black_key(key) and "piano_key_black_text" or
							    "piano_key_white_text"
							local text_color = boostio.hexToRgb(theme[text_color_key])
							boostio.drawText(
								note_name,
								10,
								row_y + vp.piano_key_height / 2.0 + 4.0,
								12,
								text_color.r,
								text_color.g,
								text_color.b,
								1.0
							)
						end

						row_y = row_y + vp.piano_key_height
					end
					current_row = current_row + 1
				end
			end
		end
	else
		for key = start_key, end_key do
			local y = boostio.pianoKeyToY(key)

			local is_black = is_black_key(key)
			local is_root = state.show_scale_highlights and boostio.isRootNote(key, state.selected_root)
			local in_scale = false
			if state.show_scale_highlights and boostio and boostio.isNoteInScale then
				local success, result = pcall(boostio.isNoteInScale, key, state.selected_scale,
					state.selected_root)
				in_scale = success and result
			end

			if is_black then
				local color = boostio.hexToRgb(theme.piano_key_black)
				boostio.drawRectangle(
					5,
					y + 1,
					piano_width - 10,
					vp.piano_key_height - 2,
					color.r,
					color.g,
					color.b,
					1.0
				)
			else
				local color = boostio.hexToRgb(theme.piano_key_white)
				boostio.drawRectangle(
					5,
					y + 1,
					piano_width - 10,
					vp.piano_key_height - 2,
					color.r,
					color.g,
					color.b,
					1.0
				)
			end

			if is_root then
				local color = boostio.hexToRgb(theme.scale_root_piano)
				boostio.drawRectangle(
					5,
					y + 1,
					piano_width - 10,
					vp.piano_key_height - 2,
					color.r,
					color.g,
					color.b,
					0.7
				)
			elseif in_scale then
				local color = boostio.hexToRgb(theme.scale_note_piano)
				boostio.drawRectangle(
					5,
					y + 1,
					piano_width - 10,
					vp.piano_key_height - 2,
					color.r,
					color.g,
					color.b,
					0.55
				)
			end

			local sep_color = boostio.hexToRgb(theme.piano_key_separator)
			boostio.drawLine(0, y, piano_width, y, sep_color.r, sep_color.g, sep_color.b, 1.0)

			if not is_black_key(key) and vp.piano_key_height >= 15.0 then
				local note_name = get_note_name(key)
				local text_color = boostio.hexToRgb(theme.piano_key_white_text)
				boostio.drawText(
					note_name,
					10,
					y + vp.piano_key_height / 2.0 + 4.0,
					12,
					text_color.r,
					text_color.g,
					text_color.b,
					1.0
				)
			end
		end
	end
end

local function render_notes(state, theme)
	local vp = state.viewport

	local has_solo = false
	for i = 1, 8 do
		if state.voice_solo[i] then
			has_solo = true
			break
		end
	end

	for i, note in ipairs(state.notes) do
		local voice = note.voice % 8

		if not state.voice_hidden[voice + 1] then
			local is_audible = true
			if has_solo then
				is_audible = state.voice_solo[voice + 1]
			else
				is_audible = not state.voice_muted[voice + 1]
			end

			local rect = get_note_rect(vp, note, state.fold_mode, state.selected_scale, state.selected_root)

			if not (rect.x + rect.width < vp.grid_x or rect.x > vp.grid_x + vp.grid_width) then
				if not (rect.y + rect.height < vp.grid_y or rect.y > vp.grid_y + vp.grid_height) then
					local voice_color = boostio.hexToRgb(theme.voice_colors[voice + 1])
					local note_color = { r = voice_color.r, g = voice_color.g, b = voice_color.b, a = 1.0 }

					local is_selected = boostio.isNoteSelected(note.id)

					if not is_audible then
						note_color.r = note_color.r * 0.3
						note_color.g = note_color.g * 0.3
						note_color.b = note_color.b * 0.3
						note_color.a = 0.5
					elseif is_selected then
						note_color.r = math.min(1.0, note_color.r * 1.3)
						note_color.g = math.min(1.0, note_color.g * 1.3)
						note_color.b = math.min(1.0, note_color.b * 1.3)
					end

					boostio.drawRoundedRectangle(
						rect.x,
						rect.y,
						rect.width,
						rect.height,
						3,
						note_color.r,
						note_color.g,
						note_color.b,
						note_color.a
					)

					if is_selected then
						boostio.strokeRoundedRectangle(rect.x, rect.y, rect.width, rect.height, 3,
							1.0, 1.0, 1.0, 1.0)
					end

					if is_audible then
						local shadow = boostio.hexToRgb(theme.note_shadow)
						boostio.drawRectangle(
							rect.x + 2,
							rect.y + rect.height - 4,
							rect.width - 4,
							3,
							shadow.r,
							shadow.g,
							shadow.b,
							theme.note_shadow_alpha
						)
					end
				end
			end
		end
	end
end

local function render_selection_box(theme)
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

local function render_measure_gutter(state, theme)
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

local function render_playhead(state, theme)
	local vp = state.viewport
	local x = boostio.msToX(state.playhead_ms)

	if x >= vp.grid_x and x <= vp.grid_x + vp.grid_width then
		local color = boostio.hexToRgb(theme.playhead)
		local win_width, win_height = boostio.getWindowSize()
		boostio.drawRectangle(x - 1, vp.grid_y, 1, win_height, color.r, color.g, color.b, theme.playhead_alpha)
		boostio.drawRectangle(x - 5, vp.grid_y - 10, 10, 10, color.r, color.g, color.b, theme.playhead_alpha)
	end
end

function piano_roll.render()
	local state = boostio.getAppState()
	local theme = config.theme

	update(state)

	render_grid(state, theme)
	render_notes(state, theme)
	render_selection_box(theme)
	render_measure_gutter(state, theme)
	render_playhead(state, theme)
	render_piano_keys(state, theme)
end

function piano_roll.on_key_down(key, shift, ctrl, alt)
	if key == "escape" then
		local selection = boostio.getSelection()
		if selection and #selection > 0 then
			boostio.clearSelection()
			return true
		end
	end
	return false
end

return piano_roll
