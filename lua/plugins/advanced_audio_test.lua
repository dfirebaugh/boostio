local M = {}

local function test_nes_noise_drums()
	print("Testing NES Noise drums...")

	boostio.addNote(0, 24, 150, {
		waveform = boostio.WAVEFORM_NES_NOISE,
		nes_noise_period = 15,
		decay = 800,
		amplitude_dbfs = -3
	})

	boostio.addNote(400, 48, 80, {
		waveform = boostio.WAVEFORM_NES_NOISE,
		nes_noise_period = 8,
		decay = 1500,
		amplitude_dbfs = -3
	})

	boostio.addNote(600, 72, 30, {
		waveform = boostio.WAVEFORM_NES_NOISE,
		nes_noise_period = 2,
		decay = 2500,
		amplitude_dbfs = -6
	})

	boostio.addNote(800, 24, 150, {
		waveform = boostio.WAVEFORM_NES_NOISE,
		nes_noise_period = 15,
		decay = 800,
		amplitude_dbfs = -3
	})

	boostio.play()
	print("Playing kick-snare-hat pattern!")
end

local function test_duty_cycle()
	print("Testing different duty cycles...")

	local note = 60
	local time = 0

	for duty = 64, 255, 64 do
		boostio.addNote(time, note, 300, {
			waveform = boostio.WAVEFORM_SQUARE,
			duty_cycle = duty,
			amplitude_dbfs = -6
		})
		time = time + 350
	end

	boostio.play()
	print("Playing square waves with 25%, 50%, 75%, 100% duty cycles")
end

local function test_decay_envelope()
	print("Testing decay envelopes...")

	boostio.addNote(0, 60, 1000, {
		waveform = boostio.WAVEFORM_SINE,
		decay = 1000,
		amplitude_dbfs = -3
	})

	boostio.addNote(1100, 60, 1000, {
		waveform = boostio.WAVEFORM_SINE,
		decay = -500,
		amplitude_dbfs = -3
	})

	boostio.play()
	print("Playing: fade out then fade in")
end

local function test_amplitude_control()
	print("Testing dBFS amplitude control...")

	local note = 69
	local time = 0

	for db = 0, -12, -3 do
		boostio.addNote(time, note, 400, {
			waveform = boostio.WAVEFORM_SINE,
			amplitude_dbfs = db
		})
		time = time + 500
	end

	boostio.play()
	print(string.format("Playing: 0dB, -3dB, -6dB, -9dB, -12dB"))
end

local function test_nes_noise_modes()
	print("Testing NES noise modes...")

	boostio.addNote(0, 60, 500, {
		waveform = boostio.WAVEFORM_NES_NOISE,
		nes_noise_period = 8,
		nes_noise_mode_flag = false,
		amplitude_dbfs = -3
	})

	boostio.addNote(600, 60, 500, {
		waveform = boostio.WAVEFORM_NES_NOISE,
		nes_noise_period = 8,
		nes_noise_mode_flag = true,
		amplitude_dbfs = -3
	})

	boostio.play()
	print("Playing: white noise then tonal noise")
end

function M.init()
	print("Advanced Audio Test plugin initializing...")

	boostio.registerCommand("test_nes_drums", test_nes_noise_drums)
	boostio.registerKeybinding("r", "test_nes_drums")

	boostio.registerCommand("test_duty", test_duty_cycle)
	boostio.registerKeybinding("t", "test_duty")

	boostio.registerCommand("test_decay", test_decay_envelope)
	boostio.registerKeybinding("y", "test_decay")

	boostio.registerCommand("test_amplitude", test_amplitude_control)
	boostio.registerKeybinding("u", "test_amplitude")

	boostio.registerCommand("test_nes_modes", test_nes_noise_modes)
	boostio.registerKeybinding("i", "test_nes_modes")

	print("Advanced Audio Test plugin initialized!")
	print("R = NES drums | T = Duty cycle | Y = Decay | U = Amplitude | I = NES modes")
end

M.init()

return M
