local M = {}

function M.init()
	print("Test Commands plugin initializing...")

	boostio.registerCommand("hello", function()
		print("Hello from Lua command!")
	end)

	boostio.registerCommand("test_zoom", function()
		print("Lua custom zoom command executed!")
	end)

	boostio.registerCommand("test_submit", function()
		print("Testing Lua API...")
		boostio.toggleSnap()
		print("Toggled snap!")
		boostio.setBpm(140)
		print("Set BPM to 140!")
		boostio.setPlayhead(2000)
		print("Set playhead to 2000ms!")
	end)

	boostio.registerKeybinding("h", "hello")
	boostio.registerKeybinding("t", "test_zoom", { ctrl = true })
	boostio.registerKeybinding("s", "test_submit", { ctrl = true, shift = true })

	print("Test Commands plugin initialized!")
end

M.init()

return M
