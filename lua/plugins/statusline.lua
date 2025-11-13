local statusline = {}

statusline.enabled = true
statusline.height = 24
statusline.bg_color = {r = 0.12, g = 0.12, b = 0.18, a = 1.0}
statusline.text_color = {r = 0.8, g = 0.85, b = 0.96, a = 1.0}

function statusline.render()
	if not statusline.enabled then
		return
	end

	local window_width = 800
	local window_height = 600
	local y_pos = window_height - statusline.height

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

	local text = "boostio - Lua plugin system active"
	boostio.drawText(
		text,
		10,
		y_pos + 4,
		14,
		statusline.text_color.r,
		statusline.text_color.g,
		statusline.text_color.b,
		statusline.text_color.a
	)
end

function statusline.toggle()
	statusline.enabled = not statusline.enabled
	return statusline.enabled
end

return statusline
