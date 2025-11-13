boostio.registerCommand("transpose_up", function()
	boostio.transposeUp()
	if voice_validation then
		voice_validation.invalidate()
	end
end)

boostio.registerCommand("transpose_down", function()
	boostio.transposeDown()
	if voice_validation then
		voice_validation.invalidate()
	end
end)
