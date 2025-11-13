local voice_controls = {}

local last_mouse_state = false
local click_handled = false

local function get_voice_state()
	local state = boostio.getAppState()
	return {
		hidden = state.voice_hidden,
		solo = state.voice_solo,
		muted = state.voice_muted,
	}
end

local function is_any_other_voice_visible(clicked_voice, voice_state)
	for v = 0, 7 do
		if v ~= clicked_voice and not voice_state.hidden[v + 1] then
			return true
		end
	end
	return false
end

local function toggle_all_other_voices(clicked_voice)
	local voice_state = get_voice_state()
	local any_other_visible = is_any_other_voice_visible(clicked_voice, voice_state)
	local hide_others = any_other_visible

	for v = 0, 7 do
		if v ~= clicked_voice then
			boostio.setVoiceHidden(v, hide_others)
		end
	end

	if toast then
		if hide_others then
			toast.info("Hidden all other voices")
		else
			toast.info("Shown all other voices")
		end
	end
end

local function toggle_single_voice(voice, row, row_type, voice_state)
	local new_value = not voice_state[row_type][voice + 1]

	if row == 0 then
		boostio.setVoiceHidden(voice, new_value)
	elseif row == 1 then
		boostio.setVoiceSolo(voice, new_value)
	elseif row == 2 then
		boostio.setVoiceMuted(voice, new_value)
	end

	if toast then
		local action_names = {
			hidden = new_value and "hidden" or "unhidden",
			solo = new_value and "solo'd" or "unsolo'd",
			muted = new_value and "muted" or "unmuted",
		}
		toast.info("Voice " .. (voice + 1) .. " " .. action_names[row_type])
	end
end

local function handle_voice_button_click(voice, row, row_type, voice_state, ctrl_held)
	if row == 0 and ctrl_held then
		toggle_all_other_voices(voice)
	else
		toggle_single_voice(voice, row, row_type, voice_state)
	end
end

function voice_controls.init()
	voice_controls.isVoiceAudible = function(voice)
		local voice_state = get_voice_state()
		local has_solo = false
		for i = 1, 8 do
			if voice_state.solo[i] then
				has_solo = true
				break
			end
		end

		if has_solo then
			return voice_state.solo[voice + 1]
		end
		return not voice_state.muted[voice + 1]
	end

	voice_controls.isVoiceVisible = function(voice)
		local voice_state = get_voice_state()
		return not voice_state.hidden[voice + 1]
	end
end

function voice_controls.on_key_down(key, shift, ctrl, alt)
	if not boostio or not boostio.setSelectedVoice or not boostio.getSelection or not boostio.setNoteVoice then
		return false
	end

	local key_to_voice = {
		["1"] = 0,
		["2"] = 1,
		["3"] = 2,
		["4"] = 3,
		["5"] = 4,
		["6"] = 5,
		["7"] = 6,
		["8"] = 7,
	}

	local voice = key_to_voice[key]
	if voice then
		local success = pcall(boostio.setSelectedVoice, voice)
		if not success then
			return false
		end

		local selection_success, selection = pcall(boostio.getSelection)
		if selection_success and selection and #selection > 0 then
			for _, note_id in ipairs(selection) do
				pcall(boostio.setNoteVoice, note_id, voice)
			end

			if toast and toast.info then
				pcall(toast.info, "Set " .. #selection .. " note(s) to voice " .. (voice + 1))
			end
		else
			if toast and toast.info then
				pcall(toast.info, "Selected voice " .. (voice + 1))
			end
		end

		return true
	end

	return false
end

function voice_controls.render()
	local theme = config.theme
	local state = boostio.getAppState()
	local voice_state = get_voice_state()
	local mx, my = boostio.getMousePosition()

	local w, h = boostio.getWindowSize()
	local panel_width = 8 * 40 + 7 * 3 + 10
	local start_x = (w - panel_width) / 2
	local start_y = 10
	local button_width = 40
	local button_height = 20
	local button_spacing = 3
	local row_spacing = 2

	local text_color = boostio.hexToRgb(theme.statusline_text)
	local bg_color = boostio.hexToRgb(theme.statusline_bg)

	local panel_height = button_height * 3 + row_spacing * 2 + 10
	boostio.drawRoundedRectangle(
		start_x - 5,
		start_y - 5,
		panel_width,
		panel_height,
		6,
		bg_color.r,
		bg_color.g,
		bg_color.b,
		theme.statusline_bg_alpha
	)

	local row_types = { "hidden", "solo", "muted" }
	local row_colors = {
		nil, -- voice colors for hidden row
		boostio.hexToRgb("#e5c890"), -- yellowish for solo
		boostio.hexToRgb("#ef9062"), -- red/orangeish for mute
	}
	local row_labels = { "", "S", "M" }

	-- Check if any voice is solo'd for mute button visual state
	local has_solo = false
	for i = 1, 8 do
		if voice_state.solo[i] then
			has_solo = true
			break
		end
	end

	for row = 0, 2 do
		local row_y = start_y + row * (button_height + row_spacing)

		for voice = 0, 7 do
			local x = start_x + voice * (button_width + button_spacing)
			local y = row_y

			local hovering = boostio.isPointInRect(mx, my, x, y, button_width, button_height)
			local is_active = voice_state[row_types[row + 1]][voice + 1]

			-- For mute row, show actual audible state
			if row == 2 then
				if has_solo then
					-- If any voice is solo'd, show as muted if this voice is not solo'd
					is_active = not voice_state.solo[voice + 1]
				else
					-- Otherwise show the actual mute state
					is_active = voice_state.muted[voice + 1]
				end
			end

			local is_selected = (row == 0) and state.selected_voice == voice

			local button_color
			local bg_r, bg_g, bg_b, bg_a

			if row == 0 then
				-- Use voice color for hidden row
				local voice_color_hex = theme.voice_colors[voice + 1]
				button_color = boostio.hexToRgb(voice_color_hex)
				bg_r, bg_g, bg_b, bg_a = button_color.r, button_color.g, button_color.b, 0.5
				if is_active then
					bg_r, bg_g, bg_b, bg_a = button_color.r * 1.2, button_color.g * 1.2, button_color.b * 1.2, 0.9
				end
			else
				-- Solo and mute buttons: grey when off, colored when on
				if is_active then
					button_color = row_colors[row + 1]
					bg_r, bg_g, bg_b, bg_a = button_color.r, button_color.g, button_color.b, 0.9
				else
					-- Grey when inactive
					bg_r, bg_g, bg_b, bg_a = 0.4, 0.4, 0.4, 0.5
				end
			end

			if hovering then
				bg_r, bg_g, bg_b = bg_r * 1.2, bg_g * 1.2, bg_b * 1.2
			end

			if is_selected then
				boostio.drawRoundedRectangle(x - 2, y - 2, button_width + 4, button_height + 4, 3, 1, 1, 1, 0.4)
			end

			boostio.drawRoundedRectangle(x, y, button_width, button_height, 3, bg_r, bg_g, bg_b, bg_a)

			local label = row_labels[row + 1] == "" and tostring(voice + 1) or row_labels[row + 1]
			local text_width = boostio.measureText(label, 11)
			boostio.drawText(
				label,
				x + (button_width - text_width) / 2,
				y + button_height / 2 + 3,
				11,
				text_color.r,
				text_color.g,
				text_color.b,
				1.0
			)
		end
	end

	local mouse_down = boostio.isMouseButtonDown(boostio.MOUSE_BUTTON_LEFT)
	local ctrl_held = boostio.isKeyDown("ctrl")

	if mouse_down and not last_mouse_state and not click_handled then
		for row = 0, 2 do
			local row_y = start_y + row * (button_height + row_spacing)

			for voice = 0, 7 do
				local x = start_x + voice * (button_width + button_spacing)
				local y = row_y

				if boostio.isPointInRect(mx, my, x, y, button_width, button_height) then
					local row_type = row_types[row + 1]
					handle_voice_button_click(voice, row, row_type, voice_state, ctrl_held)
					click_handled = true
					break
				end
			end
			if click_handled then
				break
			end
		end
	end

	if not mouse_down then
		click_handled = false
	end

	last_mouse_state = mouse_down
end

return voice_controls
