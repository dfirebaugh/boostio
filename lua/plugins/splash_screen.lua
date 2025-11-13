local splash_screen = {}

local state = {
	elapsed_ms = 0,
	active = true,
	duration_ms = 300,
	fade_in_ms = 50,
	fade_out_ms = 50,
	cached_font_size = nil,
	cached_text_width = nil,
	cached_window_width = 0,
	cached_window_height = 0,
	last_frame_time = nil,
}

function splash_screen.init(options)
	if not options then
		return
	end

	state.duration_ms = options.duration_ms or state.duration_ms
	state.fade_in_ms = options.fade_in_ms or state.fade_in_ms
	state.fade_out_ms = options.fade_out_ms or state.fade_out_ms
	state.last_frame_time = os.clock()
end

local function calculate_text_opacity()
	local fade_in = 1.0
	if state.elapsed_ms < state.fade_in_ms then
		fade_in = state.elapsed_ms / state.fade_in_ms
	end

	local fade_out = 1.0
	local fade_start_ms = state.duration_ms - state.fade_out_ms
	if state.elapsed_ms > fade_start_ms then
		local fade_elapsed = state.elapsed_ms - fade_start_ms
		fade_out = 1.0 - (fade_elapsed / state.fade_out_ms)
	end

	return fade_in * fade_out
end

local function calculate_font_size(w, h)
	local target_height = h * 0.8
	local target_width = w * 0.9
	local font_size = target_height

	local text_width = boostio.measureText("BOOSTIO", font_size)
	if text_width > target_width then
		font_size = (target_width / text_width) * font_size
	end

	return font_size
end

local function update_font_size_cache(w, h)
	if state.cached_window_width == w and state.cached_window_height == h then
		return
	end

	state.cached_font_size = calculate_font_size(w, h)
	state.cached_text_width = boostio.measureText("BOOSTIO", state.cached_font_size)
	state.cached_window_width = w
	state.cached_window_height = h
end

local function get_letter_colors(cycle_position)
	if cycle_position < 0.33 then
		local t = cycle_position / 0.33
		return 0.9, 0.4 + t * 0.5, 1.0 - t * 0.3
	end

	if cycle_position < 0.66 then
		local t = (cycle_position - 0.33) / 0.33
		return 0.9 - t * 0.3, 0.9, 0.7 - t * 0.2
	end

	local t = (cycle_position - 0.66) / 0.34
	return 0.6 + t * 0.3, 0.9 - t * 0.5, 0.5 + t * 0.5
end

function splash_screen.update()
	if not state.active then
		return
	end

	local current_time = os.clock()
	if state.last_frame_time then
		local dt_ms = (current_time - state.last_frame_time) * 1000
		state.elapsed_ms = state.elapsed_ms + dt_ms
	end
	state.last_frame_time = current_time

	if state.elapsed_ms >= state.duration_ms then
		state.active = false
	end
end

function splash_screen.render()
	if not state.active then
		return
	end

	local w, h = boostio.getWindowSize()
	update_font_size_cache(w, h)

	boostio.drawRectangle(0, 0, w, h, 0.1, 0.1, 0.12, 0.95)

	local text = "BOOSTIO"
	local font_size = state.cached_font_size or 120
	local text_width = state.cached_text_width or boostio.measureText(text, font_size)
	local opacity = calculate_text_opacity()
	local time_sec = state.elapsed_ms / 1000.0

	local start_x = w / 2 - text_width / 2
	local start_y = h / 2 + font_size / 4
	local current_x = start_x

	for i = 1, #text do
		local letter = text:sub(i, i)
		local cycle_position = (time_sec * 5.0 + i * 0.2) % 1.0
		local r, g, b = get_letter_colors(cycle_position)

		boostio.drawText(letter, current_x, start_y, font_size, r, g, b, opacity)

		local letter_width = boostio.measureText(letter, font_size)
		current_x = current_x + letter_width
	end
end

return splash_screen
