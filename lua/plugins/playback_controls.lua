local playback_controls = {}

local last_mouse_state = false
local click_handled = false
local preview_on_drag_enabled = true

function playback_controls.init() end

function playback_controls.render()
	local theme = config.theme
	local state = boostio.getAppState()
	local mx, my = boostio.getMousePosition()

	local button_height = 30
	local button_width = 50
	local button_spacing = 10
	local start_x = 10
	local button_y = 10

	local play_x = start_x
	local stop_x = play_x + button_width + button_spacing

	local preview_button_width = button_width
	local preview_button_height = 25
	local preview_button_y = button_y + button_height + 5
	local preview_x = start_x

	local play_hovering = boostio.isPointInRect(mx, my, play_x, button_y, button_width, button_height)
	local stop_hovering = boostio.isPointInRect(mx, my, stop_x, button_y, button_width, button_height)
	local preview_hovering = boostio.isPointInRect(mx, my, preview_x, preview_button_y, preview_button_width, preview_button_height)

	local bg_color = boostio.hexToRgb(theme.statusline_bg)
	local text_color = boostio.hexToRgb(theme.statusline_text)
	local green = boostio.hexToRgb(theme.statusline_highlight_on)
	local red = boostio.hexToRgb("#e78284")
	local blue = boostio.hexToRgb("#8caaee")

	local play_r, play_g, play_b = green.r, green.g, green.b
	local stop_r, stop_g, stop_b = red.r, red.g, red.b

	if play_hovering then
		play_r, play_g, play_b = play_r * 1.2, play_g * 1.2, play_b * 1.2
	end
	if stop_hovering then
		stop_r, stop_g, stop_b = stop_r * 1.2, stop_g * 1.2, stop_b * 1.2
	end

	if state.is_playing then
		boostio.drawRoundedRectangle(
			play_x,
			button_y,
			button_width,
			button_height,
			4,
			play_r * 0.7,
			play_g * 0.7,
			play_b * 0.7,
			theme.statusline_bg_alpha
		)
	else
		boostio.drawRoundedRectangle(
			play_x,
			button_y,
			button_width,
			button_height,
			4,
			play_r,
			play_g,
			play_b,
			theme.statusline_bg_alpha
		)
	end

	boostio.drawRoundedRectangle(
		stop_x,
		button_y,
		button_width,
		button_height,
		4,
		stop_r,
		stop_g,
		stop_b,
		theme.statusline_bg_alpha
	)

	local play_text = state.is_playing and "Pause" or "Play"
	local play_text_width = boostio.measureText(play_text, 12)
	boostio.drawText(
		play_text,
		play_x + (button_width - play_text_width) / 2,
		button_y + button_height / 2 + 4,
		12,
		text_color.r,
		text_color.g,
		text_color.b,
		1.0
	)

	local stop_text = "Stop"
	local stop_text_width = boostio.measureText(stop_text, 12)
	boostio.drawText(
		stop_text,
		stop_x + (button_width - stop_text_width) / 2,
		button_y + button_height / 2 + 4,
		12,
		text_color.r,
		text_color.g,
		text_color.b,
		1.0
	)

	local preview_r, preview_g, preview_b
	if preview_on_drag_enabled then
		preview_r, preview_g, preview_b = blue.r, blue.g, blue.b
	else
		preview_r, preview_g, preview_b = 0.4, 0.4, 0.4
	end

	if preview_hovering then
		preview_r, preview_g, preview_b = preview_r * 1.2, preview_g * 1.2, preview_b * 1.2
	end

	boostio.drawRoundedRectangle(
		preview_x,
		preview_button_y,
		preview_button_width,
		preview_button_height,
		4,
		preview_r,
		preview_g,
		preview_b,
		theme.statusline_bg_alpha
	)

	local preview_text = "Preview"
	local preview_text_width = boostio.measureText(preview_text, 10)
	boostio.drawText(
		preview_text,
		preview_x + (preview_button_width - preview_text_width) / 2,
		preview_button_y + preview_button_height / 2 + 3,
		10,
		text_color.r,
		text_color.g,
		text_color.b,
		1.0
	)

	local mouse_down = boostio.isMouseButtonDown(boostio.MOUSE_BUTTON_LEFT)

	if mouse_down and not last_mouse_state and not click_handled then
		if play_hovering then
			boostio.togglePlay()
			click_handled = true
		elseif stop_hovering then
			boostio.stop()
			click_handled = true
		elseif preview_hovering then
			preview_on_drag_enabled = not preview_on_drag_enabled
			click_handled = true
		end
	end

	if not mouse_down then
		click_handled = false
	end

	last_mouse_state = mouse_down
end

function playback_controls.is_preview_on_drag_enabled()
	return preview_on_drag_enabled
end

return playback_controls
