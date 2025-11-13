local instrument_selector = {}

local last_mouse_state = false
local click_handled = false

function instrument_selector.init() end

function instrument_selector.render()
	local theme = config.theme
	local state = boostio.getAppState()
	local mx, my = boostio.getMousePosition()

	local instrument_count = boostio.getInstrumentCount()

	local cols = 3
	local rows = math.ceil(instrument_count / cols)
	local button_width = 90
	local button_height = 24
	local button_spacing = 3

	local inst_panel_width = cols * button_width + (cols - 1) * button_spacing
	local inst_panel_height = rows * button_height + (rows - 1) * button_spacing

	local start_x = 491
	local start_y = 10

	local text_color = boostio.hexToRgb(theme.statusline_text)

	for i = 0, instrument_count - 1 do
		local inst = boostio.getInstrument(i)
		local col = i % cols
		local row = math.floor(i / cols)

		local x = start_x + col * (button_width + button_spacing)
		local y = start_y + row * (button_height + button_spacing)

		local hovering = boostio.isPointInRect(mx, my, x, y, button_width, button_height)
		local is_selected = (state.selected_instrument == i)

		local button_r = inst.color_r / 255
		local button_g = inst.color_g / 255
		local button_b = inst.color_b / 255
		local button_a = 0.5

		if is_selected then
			button_r = button_r * 0.7
			button_g = button_g * 0.7
			button_b = button_b * 0.7
			button_a = 0.9
		end

		if hovering then
			button_r = button_r * 1.2
			button_g = button_g * 1.2
			button_b = button_b * 1.2
		end

		boostio.drawRoundedRectangle(x, y, button_width, button_height, 3, button_r, button_g, button_b, button_a)

		local text_width = boostio.measureText(inst.name, 11)
		boostio.drawText(
			inst.name,
			x + (button_width - text_width) / 2,
			y + button_height / 2 + 3,
			11,
			text_color.r,
			text_color.g,
			text_color.b,
			1.0
		)
	end

	local mouse_down = boostio.isMouseButtonDown(boostio.MOUSE_BUTTON_LEFT)

	if mouse_down and not last_mouse_state and not click_handled then
		for i = 0, instrument_count - 1 do
			local col = i % cols
			local row = math.floor(i / cols)

			local x = start_x + col * (button_width + button_spacing)
			local y = start_y + row * (button_height + button_spacing)

			if boostio.isPointInRect(mx, my, x, y, button_width, button_height) then
				boostio.setSelectedInstrument(i)

				local selection = boostio.getSelection()
				if selection and #selection > 0 then
					for _, note_id in ipairs(selection) do
						boostio.setNoteInstrument(note_id, i)
					end

					if toast then
						local inst = boostio.getInstrument(i)
						toast.info("Set " .. #selection .. " note(s) to " .. inst.name)
					end
				else
					if toast then
						local inst = boostio.getInstrument(i)
						toast.info("Selected instrument: " .. inst.name)
					end
				end

				click_handled = true
				break
			end
		end
	end

	if not mouse_down then
		click_handled = false
	end

	last_mouse_state = mouse_down
end

return instrument_selector
