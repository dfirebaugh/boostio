boostio.registerCommand("quit", function()
	boostio.quit()
end)

boostio.registerCommand("save", function()
	local success = boostio.save()
	if toast then
		if success then
			toast.info("Saved successfully")
		else
			toast.error("Save failed")
		end
	end
end)

boostio.registerCommand("load", function()
	boostio.load()
end)
