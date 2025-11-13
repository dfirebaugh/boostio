local toast_plugin = {}

local toast_config = {
	position = "bottom_right",
	max_toasts = 3,
	default_duration = 4000,
	margin = 20,
	spacing = 10,
	width = 300,
	height = 60,
}

local toasts = {}

local function hex_to_rgb(hex)
	local hex_clean = hex:gsub("#", "")
	return {
		r = tonumber(hex_clean:sub(1, 2), 16) / 255,
		g = tonumber(hex_clean:sub(3, 4), 16) / 255,
		b = tonumber(hex_clean:sub(5, 6), 16) / 255,
	}
end

local types = nil

local function show(message, toast_type, duration)
	if #toasts >= toast_config.max_toasts then
		table.remove(toasts, 1)
	end

	local new_toast = {
		message = message,
		type = toast_type or "info",
		duration = duration or toast_config.default_duration,
		elapsed = 0,
		opacity = 1.0,
		y_offset = 50,
	}

	table.insert(toasts, new_toast)
end

local function calculate_position(index, width, height)
	local x, y = 0, 0

	if string.find(toast_config.position, "left") then
		x = toast_config.margin
	elseif string.find(toast_config.position, "center") then
		x = (width - toast_config.width) / 2
	else
		x = width - toast_config.width - toast_config.margin
	end

	if string.find(toast_config.position, "top") then
		y = toast_config.margin + index * (toast_config.height + toast_config.spacing)
	else
		y = height - toast_config.margin - (toast_config.height + toast_config.spacing) * (index + 1) - 25
	end

	return x, y
end

local function render_toast(t, index, viewport_width, viewport_height)
	local x, y = calculate_position(index, viewport_width, viewport_height)
	y = y + t.y_offset

	local type_config = types[t.type] or types.info

	boostio.drawRectangle(x + 2, y + 2, toast_config.width, toast_config.height, 0, 0, 0, t.opacity * 0.3)

	local bg = type_config.bg_color
	boostio.drawRoundedRectangle(x, y, toast_config.width, toast_config.height, 8, bg.r, bg.g, bg.b, 0.95 * t
	.opacity)

	local border = type_config.border_color
	boostio.drawRectangle(x, y, toast_config.width, 3, border.r, border.g, border.b, t.opacity)

	boostio.drawText(type_config.icon, x + 15, y + toast_config.height / 2 + 6, 18, 1, 1, 1, t.opacity)

	boostio.drawText(t.message, x + 45, y + toast_config.height / 2 + 6, 13, 1, 1, 1, t.opacity)
end

function toast_plugin.init()
	types = {
		info = {
			bg_color = hex_to_rgb(config.theme.palette.blue),
			border_color = hex_to_rgb(config.theme.palette.sapphire),
			icon = "i",
		},
		success = {
			bg_color = hex_to_rgb(config.theme.palette.green),
			border_color = hex_to_rgb(config.theme.palette.teal),
			icon = "+",
		},
		warning = {
			bg_color = hex_to_rgb(config.theme.palette.yellow),
			border_color = hex_to_rgb(config.theme.palette.peach),
			icon = "!",
		},
		error = {
			bg_color = hex_to_rgb(config.theme.palette.red),
			border_color = hex_to_rgb(config.theme.palette.maroon),
			icon = "x",
		},
	}

	toast_plugin.show = show
	toast_plugin.info = function(message, duration)
		show(message, "info", duration)
	end
	toast_plugin.success = function(message, duration)
		show(message, "success", duration)
	end
	toast_plugin.warning = function(message, duration)
		show(message, "warning", duration)
	end
	toast_plugin.error = function(message, duration)
		show(message, "error", duration)
	end
	toast_plugin.clear = function()
		toasts = {}
	end
end

function toast_plugin.render()
	if #toasts == 0 then
		return
	end

	local w, h = boostio.getWindowSize()

	local dt = 0.016
	local dt_ms = dt * 1000

	local i = 1
	while i <= #toasts do
		local t = toasts[i]
		t.elapsed = t.elapsed + dt_ms

		local fade_duration = 500
		if t.elapsed > t.duration - fade_duration then
			local fade_elapsed = t.elapsed - (t.duration - fade_duration)
			t.opacity = math.max(0, 1.0 - (fade_elapsed / fade_duration))
		end

		if t.elapsed < 200 then
			local progress = t.elapsed / 200
			t.y_offset = (1.0 - progress) * 50
		else
			t.y_offset = 0
		end

		if t.elapsed >= t.duration then
			table.remove(toasts, i)
		else
			i = i + 1
		end
	end

	for i, t in ipairs(toasts) do
		render_toast(t, i - 1, w, h)
	end
end

return toast_plugin
