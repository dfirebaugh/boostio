local piano_roll = {}

piano_roll.enabled = true

local utils = require("piano_roll.utils")
local note_ops = require("piano_roll.note_operations")
local state_module = require("piano_roll.state")
local mouse_handlers = require("piano_roll.mouse_handlers")
local rendering = require("piano_roll.rendering")

local mouse_state = state_module.create_mouse_state()

function piano_roll.init(config_options)
	state_module.init(config_options)
end

local function update(state)
	local mouse_x, mouse_y = boostio.getMousePosition()

	if boostio.isMouseButtonPressed(boostio.MOUSE_BUTTON_LEFT) then
		mouse_handlers.handle_mouse_down(
			mouse_x,
			mouse_y,
			boostio.MOUSE_BUTTON_LEFT,
			state,
			mouse_state,
			utils,
			note_ops,
			state_module.options
		)
	end

	if mouse_state.down then
		mouse_handlers.handle_mouse_move(mouse_x, mouse_y, state, mouse_state, utils, note_ops,
			state_module.options)
	end

	if not boostio.isMouseButtonDown(boostio.MOUSE_BUTTON_LEFT) and mouse_state.down then
		mouse_handlers.handle_mouse_up(
			mouse_x,
			mouse_y,
			boostio.MOUSE_BUTTON_LEFT,
			state,
			mouse_state,
			utils,
			note_ops,
			state_module,
			state_module.options
		)
	end
end

function piano_roll.render()
	local state = boostio.getAppState()
	local theme = config.theme

	update(state)

	rendering.render_grid(state, theme, state_module.options, utils)
	rendering.render_notes(state, theme, state_module.options, utils)
	rendering.render_selection_box(mouse_state)
	rendering.render_measure_gutter(state, theme)
	rendering.render_playhead(state, theme)
	rendering.render_piano_keys(state, theme, state_module.options, utils)
end

function piano_roll.on_key_down(key, shift, ctrl, alt)
	if key == "escape" then
		local selection = boostio.getSelection()
		if selection and #selection > 0 then
			boostio.clearSelection()
			return true
		end
	end
	return false
end

return piano_roll
