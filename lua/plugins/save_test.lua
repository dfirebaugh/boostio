local M = {}

function M.init()
	print("Save Test plugin initializing...")

	boostio.registerCommand("create_test_notes", function()
		print("Creating test notes...")

		boostio.setBpm(120)

		boostio.addNote(0, 0, 60, 500)
		boostio.addNote(500, 0, 64, 500)
		boostio.addNote(1000, 0, 67, 500)
		boostio.addNote(1500, 0, 72, 500)

		print("Created 4 test notes!")
	end)

	boostio.registerCommand("test_save", function()
		print("Testing save functionality...")
		print("Saving all formats (JSON, C, WAV) to current file path...")

		local success = boostio.save()
		if success then
			print("✓ All exports completed successfully!")
			print("  Files saved to the loaded file's location")
		else
			print("✗ Save failed!")
		end
	end)

	boostio.registerKeybinding("n", "create_test_notes", { ctrl = true })
	boostio.registerKeybinding("s", "test_save", { ctrl = true })

	print("Save Test plugin initialized!")
	print("  Ctrl+N: Create test notes")
	print("  Ctrl+S: Save to test_output.json")
end

M.init()

return M
