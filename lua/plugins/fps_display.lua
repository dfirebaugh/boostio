local fps_display = {}

function fps_display.render()
	local fps = boostio.getFps()
	local fps_text = string.format("FPS: %d", math.floor(fps + 0.5))
	local font_size = 16
	local text_width = boostio.measureText(fps_text, font_size)
	local window_width, window_height = boostio.getWindowSize()

	local padding = 20
	local x = window_width - text_width - padding
	local y = padding

	boostio.drawText(fps_text, x, y, font_size, 0.9, 0.9, 0.9, 0.8)
end

return fps_display
