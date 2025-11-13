boostio.registerCommand("undo", function()
	boostio.undo()
	if voice_validation then
		voice_validation.invalidate()
	end
end)

boostio.registerCommand("redo", function()
	boostio.redo()
	if voice_validation then
		voice_validation.invalidate()
	end
end)

boostio.registerCommand("delete_selected", function()
	local selection = boostio.getSelection()
	if #selection > 0 then
		for _, note_id in ipairs(selection) do
			boostio.deleteNote(note_id)
		end
		boostio.clearSelection()
		if voice_validation then
			voice_validation.invalidate()
		end
	end
end)
