local state = {}

state.default_options = {
	edge_threshold = 5,
	drag_threshold = 5,
	double_click_threshold_ms = 500,
	min_note_duration_ms = 10,
	piano_key_min = 45,
	piano_key_max = 99,
}

state.options = {}

function state.init(config_options)
	for k, v in pairs(state.default_options) do
		state.options[k] = v
	end

	if not config_options then
		return
	end

	for k, v in pairs(config_options) do
		if state.default_options[k] ~= nil then
			state.options[k] = v
		end
	end
end

function state.create_mouse_state()
	return {
		down = false,
		down_x = 0,
		down_y = 0,
		drag_mode = "none",
		drag_started = false,
		selection_box = {
			active = false,
			start_x = 0,
			start_y = 0,
			end_x = 0,
			end_y = 0,
		},
		drag_data = {
			initial_positions = {},
			initial_durations = {},
			initial_piano_keys = {},
			primary_note_id = nil,
			copied_note_ids = {},
			original_note_ids = {},
		},
		clicked_note_id = nil,
		resize_from_left = false,
		last_click_time = 0,
		last_clicked_note_id = nil,
		last_preview_piano_key = nil,
	}
end

function state.reset_mouse_state(mouse_state)
	mouse_state.down = false
	mouse_state.drag_mode = "none"
	mouse_state.drag_started = false
	mouse_state.selection_box.active = false
	mouse_state.clicked_note_id = nil
	mouse_state.last_preview_piano_key = nil
	mouse_state.drag_data.initial_positions = {}
	mouse_state.drag_data.initial_durations = {}
	mouse_state.drag_data.initial_piano_keys = {}
	mouse_state.drag_data.primary_note_id = nil
	mouse_state.drag_data.copied_note_ids = {}
	mouse_state.drag_data.original_note_ids = {}
end

return state
