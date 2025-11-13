boostio.registerCommand("toggle_snap", function()
	local enabled = boostio.toggleSnap()
	if toast then
		if enabled then
			toast.info("Grid snap enabled")
		else
			toast.info("Grid snap disabled")
		end
	end
end)

boostio.registerCommand("cycle_scale_root", function()
	local root = boostio.cycleScaleRoot()
	if toast then
		toast.info("Root: " .. root)
	end
end)

boostio.registerCommand("cycle_scale_type", function()
	local scale = boostio.cycleScaleType()
	if toast then
		toast.info("Scale: " .. scale)
	end
end)
