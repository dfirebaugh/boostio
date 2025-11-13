boostio.registerCommand("toast_test", function()
	if toast then
		toast.info("This is an info message", 2000)
		toast.success("Success! Operation completed", 2500)
		toast.warning("Warning: Check your settings", 3000)
		toast.error("Error: Something went wrong", 3500)
	else
		print("Toast plugin not loaded")
	end
end)
