local mouse_handlers = {}

local function is_in_piano_key_area(x, y, vp)
	return x >= 0 and x < vp.grid_x and y >= vp.grid_y and y < vp.grid_y + vp.grid_height
end

local function is_in_grid_area(x, y, vp)
	return x >= vp.grid_x and x <= vp.grid_x + vp.grid_width and y >= vp.grid_y and y <= vp.grid_y + vp.grid_height
end

local function get_piano_key_at_y(y, vp, state, utils, options)
	if state.fold_mode then
		return utils.y_to_piano_key_folded(y, vp, state.selected_scale, state.selected_root, options)
	end
	return boostio.yToPianoKey(y)
end

local function handle_piano_key_click(x, y, vp, state, utils, options)
	if not is_in_piano_key_area(x, y, vp) then
		return false
	end

	local piano_key = get_piano_key_at_y(y, vp, state, utils, options)
	boostio.playPreviewNote(piano_key)
	return true
end

local function init_drag_data(drag_data, store_position)
	if store_position then
		drag_data.initial_positions = {}
	end
	drag_data.initial_durations = {}
	drag_data.initial_piano_keys = {}
end

local function setup_resize_drag(mouse_state, note_id, note, state, note_ops, is_left_edge)
	mouse_state.drag_mode = is_left_edge and "resize_left" or "resize_right"
	mouse_state.resize_from_left = is_left_edge

	init_drag_data(mouse_state.drag_data, is_left_edge)

	local selection = boostio.getSelection()
	local is_selected = #selection > 0 and boostio.isNoteSelected(note_id)

	if is_selected then
		note_ops.for_each_selected_note(state, function(selected_id, note)
			note_ops.store_note_drag_data(selected_id, note, mouse_state.drag_data, is_left_edge)
		end)
		return
	end

	if note then
		note_ops.store_note_drag_data(note_id, note, mouse_state.drag_data, is_left_edge)
	end
end

local function setup_move_or_copy_drag(mouse_state, note_id, note, state, note_ops, ctrl_held, shift_held)
	mouse_state.drag_mode = shift_held and "copy" or "move"
	mouse_state.drag_data.primary_note_id = note_id
	mouse_state.drag_data.initial_positions = {}
	mouse_state.drag_data.initial_durations = {}
	mouse_state.drag_data.initial_piano_keys = {}
	mouse_state.drag_data.copied_note_ids = {}
	mouse_state.drag_data.original_note_ids = {}

	local selection = boostio.getSelection()
	local is_selected = #selection > 0 and boostio.isNoteSelected(note_id)

	if is_selected then
		if ctrl_held then
			boostio.deselectNote(note_id)
			return
		end

		note_ops.for_each_selected_note(state, function(selected_id, note)
			note_ops.store_note_drag_data(selected_id, note, mouse_state.drag_data, true)
		end)
		return
	end

	if not note or not note_ops.is_voice_visible(note.voice) then
		return
	end

	if not ctrl_held and not shift_held then
		boostio.clearSelection()
	end
	boostio.selectNote(note_id)
	note_ops.store_note_drag_data(note_id, note, mouse_state.drag_data, true)
end

local function handle_note_click(note_id, is_left_edge, is_right_edge, mouse_state, state, note_ops, ctrl_held, shift_held)
	local note = note_ops.find_note_by_id(state, note_id)
	if not note or not note_ops.is_voice_visible(note.voice) then
		return
	end

	if is_left_edge or is_right_edge then
		setup_resize_drag(mouse_state, note_id, note, state, note_ops, is_left_edge)
		return
	end

	setup_move_or_copy_drag(mouse_state, note_id, note, state, note_ops, ctrl_held, shift_held)
end

local function start_box_select(x, y, mouse_state, ctrl_held, shift_held)
	mouse_state.drag_mode = "box_select"
	mouse_state.selection_box.active = true
	mouse_state.selection_box.start_x = x
	mouse_state.selection_box.start_y = y
	mouse_state.selection_box.end_x = x
	mouse_state.selection_box.end_y = y
end

function mouse_handlers.handle_mouse_down(x, y, button, state, mouse_state, utils, note_ops, options)
	if button ~= boostio.MOUSE_BUTTON_LEFT then
		return
	end

	local vp = state.viewport

	if handle_piano_key_click(x, y, vp, state, utils, options) then
		return
	end

	if not is_in_grid_area(x, y, vp) then
		return
	end

	mouse_state.down = true
	mouse_state.down_x = x
	mouse_state.down_y = y
	mouse_state.drag_started = false

	local note_id, is_left_edge, is_right_edge = note_ops.find_note_at_position(x, y, state, utils, options)
	mouse_state.clicked_note_id = note_id

	local ctrl_held = boostio.isKeyDown("ctrl")
	local shift_held = boostio.isKeyDown("shift")

	if note_id then
		handle_note_click(note_id, is_left_edge, is_right_edge, mouse_state, state, note_ops, ctrl_held, shift_held)
		return
	end

	start_box_select(x, y, mouse_state, ctrl_held, shift_held)
end

local function create_copied_notes(mouse_state, state)
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
				local new_note_id = boostio.addNote(note.ms, note.piano_key, note.duration_ms, params)
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

local function start_copy_drag_if_needed(mouse_state, state, distance, options)
	if mouse_state.drag_started then
		return
	end

	if distance <= options.drag_threshold then
		return
	end

	mouse_state.drag_started = true

	if mouse_state.drag_mode == "copy" then
		create_copied_notes(mouse_state, state)
	end
end

local function calculate_piano_key_delta(y, down_y, state, utils, options)
	if state.fold_mode then
		local current_key = utils.y_to_piano_key_folded(y, state.viewport, state.selected_scale,
			state.selected_root, options)
		local initial_key = utils.y_to_piano_key_folded(down_y, state.viewport, state.selected_scale,
			state.selected_root, options)
		local current_row = utils.piano_key_to_folded_row(current_key, state.selected_scale,
			state.selected_root, options)
		local initial_row = utils.piano_key_to_folded_row(initial_key, state.selected_scale,
			state.selected_root, options)
		return current_row - initial_row
	end

	return boostio.yToPianoKey(y) - boostio.yToPianoKey(down_y)
end

local function calculate_snapped_delta_ms(delta_ms, mouse_state, state, note_ops, utils)
	local primary_note = nil
	if mouse_state.drag_data.primary_note_id then
		primary_note = note_ops.find_note_by_id(state, mouse_state.drag_data.primary_note_id)
	end

	if not primary_note or not state.snap_enabled then
		return delta_ms
	end

	local initial_ms = mouse_state.drag_data.initial_positions[primary_note.id]
	local target_ms = initial_ms + delta_ms
	local snapped_ms = utils.snap_to_grid(target_ms, state)
	return snapped_ms - initial_ms
end

local function calculate_new_piano_key_folded(initial_piano_key, delta_piano_key, state, utils, options)
	local initial_row = utils.piano_key_to_folded_row(initial_piano_key, state.selected_scale,
		state.selected_root, options)
	local target_row = initial_row + delta_piano_key

	local current_row = 0
	local new_key = initial_piano_key
	if not boostio or not boostio.isNoteInScale then
		return new_key
	end

	for key = options.piano_key_max, options.piano_key_min, -1 do
		local success, in_scale = pcall(boostio.isNoteInScale, key, state.selected_scale, state.selected_root)
		if success and in_scale then
			if current_row == target_row then
				return key
			end
			current_row = current_row + 1
		end
	end
	return new_key
end

local function update_note_position(note, initial_ms, initial_piano_key, delta_ms, delta_piano_key, state, utils, options, mouse_state)
	note.ms = math.max(0, initial_ms + delta_ms)

	if state.fold_mode then
		note.piano_key = calculate_new_piano_key_folded(initial_piano_key, delta_piano_key, state, utils, options)
	else
		local new_key = initial_piano_key + delta_piano_key
		note.piano_key = math.max(0, math.min(127, math.floor(new_key + 0.5)))
	end

	local should_preview = note.id == mouse_state.drag_data.primary_note_id
		and playback_controls
		and playback_controls.is_preview_on_drag_enabled
		and playback_controls.is_preview_on_drag_enabled()
		and not state.is_playing

	if should_preview and note.piano_key ~= mouse_state.last_preview_piano_key then
		boostio.playPreviewNote(note.piano_key)
		mouse_state.last_preview_piano_key = note.piano_key
	end
end

local function handle_move_or_copy_drag(x, y, mouse_state, state, note_ops, utils, options)
	local delta_ms = boostio.xToMs(x) - boostio.xToMs(mouse_state.down_x)
	local delta_piano_key = calculate_piano_key_delta(y, mouse_state.down_y, state, utils, options)

	delta_ms = calculate_snapped_delta_ms(delta_ms, mouse_state, state, note_ops, utils)

	for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
		local initial_piano_key = mouse_state.drag_data.initial_piano_keys[note_id]
		local note = note_ops.find_note_by_id(state, note_id)
		if note then
			update_note_position(note, initial_ms, initial_piano_key, delta_ms, delta_piano_key, state, utils, options, mouse_state)
		end
	end
end

local function handle_resize_left_drag(x, mouse_state, state, note_ops, utils, options)
	local delta_ms = boostio.xToMs(x) - boostio.xToMs(mouse_state.down_x)

	for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
		local initial_duration = mouse_state.drag_data.initial_durations[note_id]
		local target_ms = initial_ms + delta_ms

		if state.snap_enabled then
			target_ms = utils.snap_to_grid(target_ms, state)
			delta_ms = target_ms - initial_ms
		end

		local new_duration = initial_duration - delta_ms
		if new_duration < options.min_note_duration_ms then
			delta_ms = initial_duration - options.min_note_duration_ms
			new_duration = options.min_note_duration_ms
		end

		local note = note_ops.find_note_by_id(state, note_id)
		if note then
			note.ms = math.max(0, initial_ms + delta_ms)
			note.duration_ms = math.floor(new_duration)
		end
	end
end

local function handle_resize_right_drag(x, mouse_state, state, note_ops, utils, options)
	local delta_ms = boostio.xToMs(x) - boostio.xToMs(mouse_state.down_x)

	for note_id, initial_duration in pairs(mouse_state.drag_data.initial_durations) do
		local new_duration = initial_duration + delta_ms

		if state.snap_enabled then
			local note = note_ops.find_note_by_id(state, note_id)
			if note then
				local end_ms = note.ms + initial_duration + delta_ms
				local snapped_end_ms = utils.snap_to_grid(end_ms, state)
				new_duration = snapped_end_ms - note.ms
			end
		end

		if new_duration < options.min_note_duration_ms then
			new_duration = options.min_note_duration_ms
		end

		local note = note_ops.find_note_by_id(state, note_id)
		if note then
			note.duration_ms = math.floor(new_duration)
		end
	end
end

local function handle_box_select_drag(x, y, mouse_state, state, note_ops, utils, options)
	mouse_state.selection_box.end_x = x
	mouse_state.selection_box.end_y = y
end

local function finalize_box_select(mouse_state, state, note_ops, utils, options)
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
		local rect = utils.get_note_rect(state.viewport, note, state.fold_mode, state.selected_scale,
			state.selected_root, options)

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

		if intersects and note_ops.is_voice_visible(note.voice) then
			boostio.selectNote(note.id)
		end
	end
end

function mouse_handlers.handle_mouse_move(x, y, state, mouse_state, utils, note_ops, options)
	if not mouse_state.down then
		return
	end

	local dx = x - mouse_state.down_x
	local dy = y - mouse_state.down_y
	local distance = math.sqrt(dx * dx + dy * dy)

	start_copy_drag_if_needed(mouse_state, state, distance, options)

	if not mouse_state.drag_started then
		return
	end

	if mouse_state.drag_mode == "move" or mouse_state.drag_mode == "copy" then
		handle_move_or_copy_drag(x, y, mouse_state, state, note_ops, utils, options)
	elseif mouse_state.drag_mode == "resize_left" then
		handle_resize_left_drag(x, mouse_state, state, note_ops, utils, options)
	elseif mouse_state.drag_mode == "resize_right" then
		handle_resize_right_drag(x, mouse_state, state, note_ops, utils, options)
	elseif mouse_state.drag_mode == "box_select" then
		handle_box_select_drag(x, y, mouse_state, state, note_ops, utils, options)
	end
end

local function finalize_move_or_copy(mouse_state, state, note_ops)
	for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
		local initial_piano_key = mouse_state.drag_data.initial_piano_keys[note_id]
		local note = note_ops.find_note_by_id(state, note_id)
		if note then
			local delta_ms = math.floor(note.ms - initial_ms)
			local delta_piano_key = math.floor(note.piano_key - initial_piano_key)
			boostio.moveNote(note_id, delta_ms, delta_piano_key)
		end
	end
end

local function finalize_resize_left(mouse_state, state, note_ops)
	for note_id, initial_ms in pairs(mouse_state.drag_data.initial_positions) do
		local initial_duration = mouse_state.drag_data.initial_durations[note_id]
		local note = note_ops.find_note_by_id(state, note_id)
		if note then
			local delta_duration = math.floor(note.duration_ms - initial_duration)
			boostio.resizeNote(note_id, delta_duration, true)
		end
	end
end

local function finalize_resize_right(mouse_state, state, note_ops)
	for note_id, initial_duration in pairs(mouse_state.drag_data.initial_durations) do
		local note = note_ops.find_note_by_id(state, note_id)
		if note then
			local delta_duration = math.floor(note.duration_ms - initial_duration)
			boostio.resizeNote(note_id, delta_duration, false)
		end
	end
end

local function finalize_drag(mouse_state, state, note_ops, utils, options)
	if mouse_state.drag_mode == "box_select" then
		finalize_box_select(mouse_state, state, note_ops, utils, options)
		return
	end

	if not mouse_state.drag_started then
		return
	end

	if mouse_state.drag_mode == "move" or mouse_state.drag_mode == "copy" then
		finalize_move_or_copy(mouse_state, state, note_ops)
	elseif mouse_state.drag_mode == "resize_left" then
		finalize_resize_left(mouse_state, state, note_ops)
	elseif mouse_state.drag_mode == "resize_right" then
		finalize_resize_right(mouse_state, state, note_ops)
	end
end

local function handle_double_click(note_id, mouse_state)
	boostio.deleteNote(note_id)
	mouse_state.last_click_time = 0
	mouse_state.last_clicked_note_id = nil
end

local function handle_single_click_on_note(note_id, note, state, mouse_state, ctrl_held, shift_held)
	mouse_state.last_click_time = os.clock() * 1000
	mouse_state.last_clicked_note_id = note_id

	local should_preview = playback_controls
		and playback_controls.is_preview_on_drag_enabled
		and playback_controls.is_preview_on_drag_enabled()
		and not state.is_playing

	if should_preview then
		boostio.playPreviewNote(note.piano_key)
	end

	if ctrl_held then
		if boostio.isNoteSelected(note_id) then
			boostio.deselectNote(note_id)
		else
			boostio.selectNote(note_id)
		end
		return
	end

	if shift_held then
		boostio.selectNote(note_id)
		return
	end

	boostio.clearSelection()
	boostio.selectNote(note_id)
end

local function handle_click_on_note(note_id, state, mouse_state, note_ops, ctrl_held, shift_held, options)
	local current_time = os.clock() * 1000
	local time_since_last_click = current_time - mouse_state.last_click_time
	local is_double_click = (time_since_last_click < options.double_click_threshold_ms)
		and (note_id == mouse_state.last_clicked_note_id)

	if is_double_click then
		handle_double_click(note_id, mouse_state)
		return
	end

	local note = note_ops.find_note_by_id(state, note_id)
	if not note or not note_ops.is_voice_visible(note.voice) then
		return
	end

	handle_single_click_on_note(note_id, note, state, mouse_state, ctrl_held, shift_held)
end

local function handle_click_on_empty_space(x, y, vp, state, mouse_state, note_ops, utils, ctrl_held, shift_held, options)
	mouse_state.last_click_time = 0
	mouse_state.last_clicked_note_id = nil

	if not ctrl_held and not shift_held then
		boostio.clearSelection()
	end

	if not is_in_grid_area(x, y, vp) then
		return
	end

	local ms = boostio.xToMs(x)
	local piano_key = get_piano_key_at_y(y, vp, state, utils, options)

	if state.snap_enabled and state.snap_ms > 0 then
		ms = math.floor(ms / state.snap_ms + 0.5) * state.snap_ms
	end

	ms = math.max(0, ms)

	if piano_key < options.piano_key_min or piano_key > options.piano_key_max then
		return
	end

	local ms_per_beat = 60000.0 / state.bpm
	local default_duration = ms_per_beat

	local new_note_id = boostio.addNote(ms, piano_key, default_duration)
	if not new_note_id then
		return
	end

	local new_note = note_ops.find_note_by_id(state, new_note_id)
	if not new_note or not note_ops.is_voice_visible(new_note.voice) then
		return
	end

	boostio.clearSelection()
	boostio.selectNote(new_note_id)
end

local function handle_click(x, y, vp, state, mouse_state, note_ops, utils, ctrl_held, shift_held, options)
	local note_id = mouse_state.clicked_note_id

	if note_id then
		handle_click_on_note(note_id, state, mouse_state, note_ops, ctrl_held, shift_held, options)
		return
	end

	handle_click_on_empty_space(x, y, vp, state, mouse_state, note_ops, utils, ctrl_held, shift_held, options)
end

function mouse_handlers.handle_mouse_up(x, y, button, state, mouse_state, utils, note_ops, state_module, options)
	if button ~= boostio.MOUSE_BUTTON_LEFT then
		return
	end

	finalize_drag(mouse_state, state, note_ops, utils, options)

	if mouse_state.down and not mouse_state.drag_started then
		local vp = state.viewport
		local ctrl_held = boostio.isKeyDown("ctrl")
		local shift_held = boostio.isKeyDown("shift")

		handle_click(x, y, vp, state, mouse_state, note_ops, utils, ctrl_held, shift_held, options)
	end

	if voice_validation then
		voice_validation.invalidate()
	end

	state_module.reset_mouse_state(mouse_state)
end

return mouse_handlers
