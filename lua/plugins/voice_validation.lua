local voice_validation = {}

voice_validation.enabled = true

local validation_errors = {}
local hover_error = nil

local function is_voice_visible(voice)
	if voice_controls and voice_controls.isVoiceVisible then
		return voice_controls.isVoiceVisible(voice)
	end
	return true
end

local function detect_voice_overlaps(notes)
	local errors = {}

	local notes_by_voice = {}
	for i = 1, 8 do
		notes_by_voice[i] = {}
	end

	for _, note in ipairs(notes) do
		local voice = note.voice % 8
		if is_voice_visible(voice) then
			table.insert(notes_by_voice[voice + 1], note)
		end
	end

	for voice = 1, 8 do
		local voice_notes = notes_by_voice[voice]

		if #voice_notes > 1 then
			table.sort(voice_notes, function(a, b)
				return a.ms < b.ms
			end)

			local processed = {}
			for i = 1, #voice_notes do
				if not processed[i] then
					local note = voice_notes[i]
					local note_end = note.ms + note.duration_ms
					local conflict_notes = {note.id}
					local conflict_indices = {i}
					local conflict_end = note_end

					for j = i + 1, #voice_notes do
						local check_note = voice_notes[j]
						local check_end = check_note.ms + check_note.duration_ms

						if check_note.ms < conflict_end then
							table.insert(conflict_notes, check_note.id)
							table.insert(conflict_indices, j)
							conflict_end = math.max(conflict_end, check_end)
						else
							break
						end
					end

					if #conflict_notes > 1 then
						for _, idx in ipairs(conflict_indices) do
							processed[idx] = true
						end

						local error_data = {
							voice = voice - 1,
							start_ms = note.ms,
							end_ms = conflict_end,
							note_ids = conflict_notes,
						}
						table.insert(errors, error_data)
					end
				end
			end
		end
	end

	return errors
end

local function render_validation_highlights(state, theme)
	if #validation_errors == 0 then
		return
	end

	local vp = state.viewport

	for _, error in ipairs(validation_errors) do
		local start_x = boostio.msToX(error.start_ms)
		local end_x = boostio.msToX(error.end_ms)
		local width = end_x - start_x

		if start_x < vp.grid_x + vp.grid_width and end_x > vp.grid_x then
			boostio.drawRectangle(
				start_x,
				vp.grid_y,
				width,
				vp.grid_height,
				0.8,
				0.2,
				0.2,
				0.15
			)

			boostio.drawLine(
				start_x,
				vp.grid_y,
				start_x,
				vp.grid_y + vp.grid_height,
				1.0,
				0.3,
				0.3,
				0.5
			)

			boostio.drawLine(
				end_x,
				vp.grid_y,
				end_x,
				vp.grid_y + vp.grid_height,
				1.0,
				0.3,
				0.3,
				0.5
			)
		end
	end
end

local function render_error_indicators(state, theme)
	if #validation_errors == 0 then
		return
	end

	local vp = state.viewport
	local indicator_size = 18
	local gutter_y = vp.grid_y - indicator_size - 5

	local mouse_x, mouse_y = boostio.getMousePosition()
	hover_error = nil

	for _, error in ipairs(validation_errors) do
		local start_x = boostio.msToX(error.start_ms)
		local end_x = boostio.msToX(error.end_ms)
		local center_x = (start_x + end_x) / 2

		if center_x >= vp.grid_x and center_x <= vp.grid_x + vp.grid_width then
			local indicator_x = center_x - indicator_size / 2
			local indicator_y = gutter_y

			local error_color = boostio.hexToRgb(theme.palette.red)

			boostio.drawRoundedRectangle(
				indicator_x,
				indicator_y,
				indicator_size,
				indicator_size,
				4,
				error_color.r,
				error_color.g,
				error_color.b,
				1.0
			)

			boostio.drawText(
				"E",
				indicator_x + indicator_size / 2 - 4,
				indicator_y + indicator_size / 2 + 5,
				12,
				1.0,
				1.0,
				1.0,
				1.0
			)

			local is_hover = boostio.isPointInRect(
				mouse_x,
				mouse_y,
				indicator_x,
				indicator_y,
				indicator_size,
				indicator_size
			)

			if is_hover then
				hover_error = error
			end
		end
	end

	if hover_error then
		local tooltip_width = 300
		local tooltip_height = 60
		local tooltip_x = mouse_x + 10
		local tooltip_y = mouse_y + 10

		local win_width, win_height = boostio.getWindowSize()
		if tooltip_x + tooltip_width > win_width then
			tooltip_x = mouse_x - tooltip_width - 10
		end
		if tooltip_y + tooltip_height > win_height then
			tooltip_y = mouse_y - tooltip_height - 10
		end

		local bg_color = boostio.hexToRgb(theme.palette.surface0)
		boostio.drawRoundedRectangle(
			tooltip_x + 2,
			tooltip_y + 2,
			tooltip_width,
			tooltip_height,
			8,
			0.0,
			0.0,
			0.0,
			0.3
		)

		boostio.drawRoundedRectangle(
			tooltip_x,
			tooltip_y,
			tooltip_width,
			tooltip_height,
			8,
			bg_color.r,
			bg_color.g,
			bg_color.b,
			0.95
		)

		local error_color = boostio.hexToRgb(theme.palette.red)
		boostio.drawRectangle(
			tooltip_x,
			tooltip_y,
			tooltip_width,
			4,
			error_color.r,
			error_color.g,
			error_color.b,
			1.0
		)

		local text_color = boostio.hexToRgb(theme.palette.text)
		local title = string.format("Voice %d: Overlapping Notes", hover_error.voice)
		boostio.drawText(
			title,
			tooltip_x + 12,
			tooltip_y + 22,
			13,
			text_color.r,
			text_color.g,
			text_color.b,
			1.0
		)

		local message = "Only one note per voice can play at a time."
		boostio.drawText(
			message,
			tooltip_x + 12,
			tooltip_y + 42,
			11,
			text_color.r * 0.8,
			text_color.g * 0.8,
			text_color.b * 0.8,
			1.0
		)
	end
end

function voice_validation.render()
	local state = boostio.getAppState()
	local theme = config.theme

	validation_errors = detect_voice_overlaps(state.notes)

	render_validation_highlights(state, theme)
	render_error_indicators(state, theme)
end

return voice_validation
