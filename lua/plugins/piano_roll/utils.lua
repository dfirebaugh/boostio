local utils = {}

function utils.is_black_key(piano_key)
	local note = piano_key % 12
	return (note == 1 or note == 3 or note == 6 or note == 8 or note == 10)
end

function utils.get_note_name(piano_key)
	local names = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }
	local note = piano_key % 12
	local octave = math.floor(piano_key / 12) - 1
	return names[note + 1] .. octave
end

function utils.piano_key_to_folded_row(piano_key, scale, root, options)
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

function utils.y_to_piano_key_folded(y, vp, scale, root, options)
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

function utils.get_note_rect(vp, note, fold_mode, scale, root, options)
	local x = boostio.msToX(note.ms)
	local y

	if fold_mode then
		local folded_row = utils.piano_key_to_folded_row(note.piano_key, scale, root, options)
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

function utils.point_in_rect(px, py, rect)
	return px >= rect.x and px <= rect.x + rect.width and py >= rect.y and py <= rect.y + rect.height
end

function utils.snap_to_grid(ms, state)
	if not state.snap_enabled then
		return ms
	end

	local ms_per_beat = 60000.0 / state.bpm
	local ms_per_32nd = ms_per_beat / 8.0
	local grid_index = math.floor(ms / ms_per_32nd + 0.5)
	return grid_index * ms_per_32nd
end

return utils
