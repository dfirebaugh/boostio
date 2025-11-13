boostio.registerCommand("zoom_in", function()
	local mouse_x, mouse_y = boostio.getMousePosition()
	boostio.zoomHorizontalAtMouse(1.2, mouse_x)
	boostio.zoomVerticalAtMouse(1.2, mouse_y)
end)

boostio.registerCommand("zoom_out", function()
	local mouse_x, mouse_y = boostio.getMousePosition()
	boostio.zoomHorizontalAtMouse(1.0 / 1.2, mouse_x)
	boostio.zoomVerticalAtMouse(1.0 / 1.2, mouse_y)
end)

boostio.registerCommand("toggle_fold", function()
	local enabled = boostio.toggleFold()
	if toast then
		if enabled then
			toast.info("Fold mode enabled")
		else
			toast.info("Fold mode disabled")
		end
	end
end)

boostio.registerCommand("toggle_scale_highlighting", function()
	local enabled = boostio.toggleScaleHighlight()
	if toast then
		if enabled then
			toast.info("Scale highlighting enabled")
		else
			toast.info("Scale highlighting disabled")
		end
	end
end)
