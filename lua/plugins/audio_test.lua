local audio_test = {}

local function play_scale()
	print("Playing C major scale...")

	local notes = { 60, 62, 64, 65, 67, 69, 71, 72 }
	local time_ms = 0

	for i, pitch in ipairs(notes) do
		boostio.addNote(time_ms, pitch, 400, boostio.WAVEFORM_SINE)
		time_ms = time_ms + 500
	end

	boostio.play()
end

local function play_chord()
	print("Playing C major chord...")

	boostio.addNote(0, 60, 2000, boostio.WAVEFORM_SINE)
	boostio.addNote(0, 64, 2000, boostio.WAVEFORM_SINE)
	boostio.addNote(0, 67, 2000, boostio.WAVEFORM_SINE)

	boostio.play()
end

local function play_waveforms()
	print("Playing different waveforms...")

	local pitch = 69
	boostio.addNote(0, pitch, 500, boostio.WAVEFORM_SINE)
	boostio.addNote(600, pitch, 500, boostio.WAVEFORM_SQUARE)
	boostio.addNote(1200, pitch, 500, boostio.WAVEFORM_TRIANGLE)
	boostio.addNote(1800, pitch, 500, boostio.WAVEFORM_SAWTOOTH)

	boostio.play()
end

function audio_test.init()
	print("Audio Test plugin initializing...")

	boostio.registerCommand("play_scale", play_scale)
	boostio.registerKeybinding("q", "play_scale")

	boostio.registerCommand("play_chord", play_chord)
	boostio.registerKeybinding("w", "play_chord")

	boostio.registerCommand("play_waveforms", play_waveforms)
	boostio.registerKeybinding("e", "play_waveforms")

	print("Audio Test plugin initialized!")
	print("Press Q for scale, W for chord, E for waveforms")
end

audio_test.init()

return audio_test
