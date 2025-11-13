boostio.registerCommand("toggle_play", function()
	boostio.togglePlay()
end)

boostio.registerCommand("play", function()
	boostio.play()
end)

boostio.registerCommand("pause", function()
	boostio.pause()
end)

boostio.registerCommand("stop", function()
	boostio.stop()
end)

local function adjust_bpm(amount)
	local state = boostio.getAppState()
	local new_bpm = state.bpm + amount
	if new_bpm < 1 then
		new_bpm = 1
	elseif new_bpm > 300 then
		new_bpm = 300
	end
	boostio.setBpm(new_bpm)
end

boostio.registerCommand("increase_bpm", function()
	adjust_bpm(1)
end)

boostio.registerCommand("decrease_bpm", function()
	adjust_bpm(-1)
end)

boostio.registerCommand("increase_bpm_5", function()
	adjust_bpm(5)
end)

boostio.registerCommand("decrease_bpm_5", function()
	adjust_bpm(-5)
end)

boostio.registerCommand("increase_bpm_10", function()
	adjust_bpm(10)
end)

boostio.registerCommand("decrease_bpm_10", function()
	adjust_bpm(-10)
end)
