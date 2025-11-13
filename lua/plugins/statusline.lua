local statusline = {}

statusline.enabled = true
statusline.height = 25

local theme = config.theme
local bg_color_rgb = common.boostio.hexToRgb(theme.statusline_bg)
statusline.bg_color = { r = bg_color_rgb.r, g = bg_color_rgb.g, b = bg_color_rgb.b, a = theme.statusline_bg_alpha }
statusline.text_color_rgb = boostio.hexToRgb(theme.statusline_text)
statusline.text_color =
	{ r = statusline.text_color_rgb.r, g = statusline.text_color_rgb.g, b = statusline.text_color_rgb.b, a = 1.0 }
statusline.separator_color_rgb = boostio.hexToRgb(theme.statusline_separator)
statusline.separator_color = {
	r = statusline.separator_color_rgb.r,
	g = statusline.separator_color_rgb.g,
	b = statusline.separator_color_rgb.b,
	a = 0.8,
}

local voice_colors = {}
for i, hex in ipairs(theme.voice_colors) do
	local rgb = boostio.hexToRgb(hex)
	voice_colors[i] = { r = rgb.r, g = rgb.g, b = rgb.b, a = 0.9 }
end

local function draw_group(items, x, y, height)
	local current_x = x
	local item_spacing = 2

	for i, item in ipairs(items) do
		if item then
			local text_width = boostio.measureText(item.text, 11)
			local padding = 8
			local total_width = text_width + padding * 2

			if item.bg_color then
				boostio.drawRoundedRectangle(
					current_x,
					y + 2,
					total_width,
					height - 4,
					2,
					item.bg_color.r,
					item.bg_color.g,
					item.bg_color.b,
					item.bg_color.a
				)
			end

			boostio.drawText(
				item.text,
				current_x + padding,
				y + 17,
				11,
				item.color.r,
				item.color.g,
				item.color.b,
				item.color.a or 1.0
			)

			current_x = current_x + total_width + item_spacing
		end
	end

	return current_x
end

local function get_group_width(items)
	local width = 0
	local item_spacing = 2

	for i, item in ipairs(items) do
		if item then
			local text_width = boostio.measureText(item.text, 11)
			local padding = 8
			width = width + text_width + padding * 2
			if i < #items then
				width = width + item_spacing
			end
		end
	end

	return width
end

local function create_bpm_component(state)
	local bpm_color_rgb = boostio.hexToRgb(theme.statusline_bpm)
	return {
		text = string.format("%d BPM", state.bpm),
		color = { r = bpm_color_rgb.r, g = bpm_color_rgb.g, b = bpm_color_rgb.b, a = 1.0 },
	}
end

local function create_voice_component(state)
	local voice_color = voice_colors[(state.selected_voice % 8) + 1]
	return {
		text = string.format("Voice %d", state.selected_voice + 1),
		color = voice_color,
		bg_color = { r = voice_color.r * 0.3, g = voice_color.g * 0.3, b = voice_color.b * 0.3, a = 0.8 },
	}
end

local function create_waveform_component(state)
	local icon = "?"
	if state.waveform == "square" then
		icon = "[SQ]"
	elseif state.waveform == "triangle" then
		icon = "[TRI]"
	elseif state.waveform == "sawtooth" then
		icon = "[SAW]"
	elseif state.waveform == "sine" then
		icon = "[SIN]"
	elseif state.waveform == "nes_noise" then
		icon = "[NES]"
	end

	return {
		text = icon .. " " .. state.waveform,
		color = statusline.text_color,
	}
end

local function create_playback_component(state)
	local icon = "[STOP]"
	local color = { r = 0.5, g = 0.5, b = 0.5, a = 1.0 }

	if state.is_playing then
		icon = "[PLAY]"
		color = { r = 0.2, g = 0.8, b = 0.2, a = 1.0 }
	end

	local seconds = math.floor(state.playhead_ms / 1000)
	local minutes = math.floor(seconds / 60)
	seconds = seconds % 60
	local time_text = string.format("%02d:%02d", minutes, seconds)

	return {
		text = icon .. " " .. time_text,
		color = color,
	}
end

local function create_notes_component(state)
	return {
		text = string.format("%d notes", state.note_count),
		color = statusline.text_color,
	}
end

local function create_scale_component(state)
	local scale_info = boostio.getScaleInfo()
	return {
		text = string.format("%s %s", scale_info.root, scale_info.scale),
		color = statusline.text_color,
	}
end

local function create_highlight_component(state)
	local scale_info = boostio.getScaleInfo()
	local text = scale_info.highlight and "[H:ON]" or "[H:OFF]"
	local color_rgb
	if scale_info.highlight then
		color_rgb = boostio.hexToRgb(theme.statusline_highlight_on)
	else
		color_rgb = boostio.hexToRgb(theme.statusline_highlight_off)
	end

	return {
		text = text,
		color = { r = color_rgb.r, g = color_rgb.g, b = color_rgb.b },
	}
end

local function create_snap_component(state)
	if not state.snap_enabled then
		return nil
	end

	return {
		text = string.format("[S:%dms]", state.snap_ms),
		color = { r = 0.7, g = 0.7, b = 0.9, a = 1.0 },
	}
end

function statusline.render()
	if not statusline.enabled then
		return
	end

	local window_width, window_height = boostio.getWindowSize()
	local y_pos = window_height - statusline.height
	local state = boostio.getAppState()

	boostio.drawRectangle(
		0,
		y_pos,
		window_width,
		statusline.height,
		statusline.bg_color.r,
		statusline.bg_color.g,
		statusline.bg_color.b,
		statusline.bg_color.a
	)

	boostio.drawLine(
		0,
		y_pos,
		window_width,
		y_pos,
		statusline.separator_color.r,
		statusline.separator_color.g,
		statusline.separator_color.b,
		statusline.separator_color.a
	)

	local group_spacing = 25

	local left_groups = {
		{ create_bpm_component(state), create_voice_component(state), create_waveform_component(state) },
	}

	local center_groups = {
		{ create_playback_component(state) },
	}

	local right_groups = {
		{ create_notes_component(state) },
		{ create_scale_component(state), create_highlight_component(state), create_snap_component(state) },
	}

	local current_x = 15
	for i, group in ipairs(left_groups) do
		if i > 1 then
			boostio.drawText(
				"|",
				current_x - group_spacing / 2 - 4,
				y_pos + 17,
				12,
				statusline.separator_color.r,
				statusline.separator_color.g,
				statusline.separator_color.b,
				statusline.separator_color.a
			)
		end
		current_x = draw_group(group, current_x, y_pos, statusline.height)
		current_x = current_x + group_spacing
	end

	local total_center_width = 0
	for _, group in ipairs(center_groups) do
		total_center_width = total_center_width + get_group_width(group)
	end
	total_center_width = total_center_width + (#center_groups - 1) * group_spacing

	local center_x = (window_width - total_center_width) / 2
	for _, group in ipairs(center_groups) do
		center_x = draw_group(group, center_x, y_pos, statusline.height)
		center_x = center_x + group_spacing
	end

	local total_right_width = 0
	for i, group in ipairs(right_groups) do
		total_right_width = total_right_width + get_group_width(group)
		if i < #right_groups then
			total_right_width = total_right_width + group_spacing
		end
	end

	local right_x = window_width - total_right_width - 15
	for i, group in ipairs(right_groups) do
		if i > 1 then
			boostio.drawText(
				"|",
				right_x - group_spacing / 2 - 4,
				y_pos + 17,
				12,
				statusline.separator_color.r,
				statusline.separator_color.g,
				statusline.separator_color.b,
				statusline.separator_color.a
			)
		end
		right_x = draw_group(group, right_x, y_pos, statusline.height)
		right_x = right_x + group_spacing
	end
end

function statusline.toggle()
	statusline.enabled = not statusline.enabled
	return statusline.enabled
end

return statusline
