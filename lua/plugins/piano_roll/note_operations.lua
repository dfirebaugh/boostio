local note_operations = {}

function note_operations.find_note_by_id(state, note_id)
	for _, note in ipairs(state.notes) do
		if note.id == note_id then
			return note
		end
	end
	return nil
end

function note_operations.for_each_selected_note(state, callback)
	local selection = boostio.getSelection()
	for _, selected_id in ipairs(selection) do
		local note = note_operations.find_note_by_id(state, selected_id)
		if note then
			callback(selected_id, note)
		end
	end
end

function note_operations.store_note_drag_data(note_id, note, drag_data, store_position)
	if store_position then
		drag_data.initial_positions[note_id] = note.ms
	end
	drag_data.initial_durations[note_id] = note.duration_ms
	drag_data.initial_piano_keys[note_id] = note.piano_key
end

function note_operations.is_voice_visible(voice)
	if voice_controls and voice_controls.isVoiceVisible then
		return voice_controls.isVoiceVisible(voice)
	end
	return true
end

function note_operations.find_note_at_position(x, y, state, utils, options)
	for i = #state.notes, 1, -1 do
		local note = state.notes[i]
		local rect = utils.get_note_rect(state.viewport, note, state.fold_mode, state.selected_scale,
			state.selected_root, options)

		if utils.point_in_rect(x, y, rect) then
			local is_left_edge = (x - rect.x) <= options.edge_threshold
			local is_right_edge = (rect.x + rect.width - x) <= options.edge_threshold

			return note.id, is_left_edge, is_right_edge
		end
	end
	return nil, false, false
end

return note_operations
