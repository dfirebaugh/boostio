boostio.registerCommand("undo", function()
	boostio.undo()
end)

boostio.registerCommand("redo", function()
	boostio.redo()
end)

boostio.registerCommand("delete_selected", function()
	local selection = boostio.getSelection()
	if #selection > 0 then
		for _, note_id in ipairs(selection) do
			boostio.deleteNote(note_id)
		end
		boostio.clearSelection()
	end
end)
