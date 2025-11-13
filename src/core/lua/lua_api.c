#include "core/lua/lua_api.h"
#include "app/app_state.h"
#include "app/lua_command_registry.h"
#include "core/audio/audio.h"
#include "core/audio/scale.h"
#include "core/audio/sequencer.h"
#include "core/audio/synth.h"
#include "core/graphics/color.h"
#include "core/graphics/graphics.h"
#include "core/input/input_types.h"

#include <string.h>

static struct lua_api_context *global_context = NULL;

static int lua_api_draw_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	int x = (int)luaL_checknumber(L, 1);
	int y = (int)luaL_checknumber(L, 2);
	int width = (int)luaL_checknumber(L, 3);
	int height = (int)luaL_checknumber(L, 4);
	float r = (float)luaL_checknumber(L, 5);
	float g = (float)luaL_checknumber(L, 6);
	float b = (float)luaL_checknumber(L, 7);
	float a = (float)luaL_optnumber(L, 8, 1.0);

	struct Color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rect(global_context->graphics, x, y, width, height);

	return 0;
}

static int lua_api_draw_text(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	const char *text = luaL_checkstring(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int size = (int)luaL_checknumber(L, 4);
	float r = (float)luaL_checknumber(L, 5);
	float g = (float)luaL_checknumber(L, 6);
	float b = (float)luaL_checknumber(L, 7);
	float a = (float)luaL_optnumber(L, 8, 1.0);

	struct Color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_text(global_context->graphics, text, x, y, size);

	return 0;
}

static int lua_api_draw_rounded_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	int x = (int)luaL_checknumber(L, 1);
	int y = (int)luaL_checknumber(L, 2);
	int width = (int)luaL_checknumber(L, 3);
	int height = (int)luaL_checknumber(L, 4);
	int radius = (int)luaL_checknumber(L, 5);
	float r = (float)luaL_checknumber(L, 6);
	float g = (float)luaL_checknumber(L, 7);
	float b = (float)luaL_checknumber(L, 8);
	float a = (float)luaL_optnumber(L, 9, 1.0);

	struct Color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rounded_rect(global_context->graphics, x, y, width, height, radius);

	return 0;
}

static int lua_api_draw_rectangle_outlined(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	int x = (int)luaL_checknumber(L, 1);
	int y = (int)luaL_checknumber(L, 2);
	int width = (int)luaL_checknumber(L, 3);
	int height = (int)luaL_checknumber(L, 4);
	float r = (float)luaL_checknumber(L, 5);
	float g = (float)luaL_checknumber(L, 6);
	float b = (float)luaL_checknumber(L, 7);
	float a = (float)luaL_optnumber(L, 8, 1.0);
	float or = (float)luaL_checknumber(L, 9);
	float og = (float)luaL_checknumber(L, 10);
	float ob = (float)luaL_checknumber(L, 11);
	float oa = (float)luaL_optnumber(L, 12, 1.0);
	int outline_width = (int)luaL_checknumber(L, 13);

	struct Color color = color_from_floats(r, g, b, a);
	struct Color outline_color = color_from_floats(or, og, ob, oa);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rect_outlined(
			global_context->graphics, x, y, width, height, outline_color, outline_width
	);

	return 0;
}

static int lua_api_stroke_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	int x = (int)luaL_checknumber(L, 1);
	int y = (int)luaL_checknumber(L, 2);
	int width = (int)luaL_checknumber(L, 3);
	int height = (int)luaL_checknumber(L, 4);
	float r = (float)luaL_checknumber(L, 5);
	float g = (float)luaL_checknumber(L, 6);
	float b = (float)luaL_checknumber(L, 7);
	float a = (float)luaL_optnumber(L, 8, 1.0);

	struct Color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_rect(global_context->graphics, x, y, width, height);

	return 0;
}

static int lua_api_draw_rounded_rectangle_outlined(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	int x = (int)luaL_checknumber(L, 1);
	int y = (int)luaL_checknumber(L, 2);
	int width = (int)luaL_checknumber(L, 3);
	int height = (int)luaL_checknumber(L, 4);
	int radius = (int)luaL_checknumber(L, 5);
	float r = (float)luaL_checknumber(L, 6);
	float g = (float)luaL_checknumber(L, 7);
	float b = (float)luaL_checknumber(L, 8);
	float a = (float)luaL_optnumber(L, 9, 1.0);
	float or = (float)luaL_checknumber(L, 10);
	float og = (float)luaL_checknumber(L, 11);
	float ob = (float)luaL_checknumber(L, 12);
	float oa = (float)luaL_optnumber(L, 13, 1.0);
	int outline_width = (int)luaL_checknumber(L, 14);

	struct Color color = color_from_floats(r, g, b, a);
	struct Color outline_color = color_from_floats(or, og, ob, oa);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rounded_rect_outlined(
			global_context->graphics, x, y, width, height, radius, outline_color, outline_width
	);

	return 0;
}

static int lua_api_stroke_rounded_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	int x = (int)luaL_checknumber(L, 1);
	int y = (int)luaL_checknumber(L, 2);
	int width = (int)luaL_checknumber(L, 3);
	int height = (int)luaL_checknumber(L, 4);
	int radius = (int)luaL_checknumber(L, 5);
	float r = (float)luaL_checknumber(L, 6);
	float g = (float)luaL_checknumber(L, 7);
	float b = (float)luaL_checknumber(L, 8);
	float a = (float)luaL_optnumber(L, 9, 1.0);

	struct Color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_rounded_rect(global_context->graphics, x, y, width, height, radius);

	return 0;
}

static int lua_api_draw_line(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	int x1 = (int)luaL_checknumber(L, 1);
	int y1 = (int)luaL_checknumber(L, 2);
	int x2 = (int)luaL_checknumber(L, 3);
	int y2 = (int)luaL_checknumber(L, 4);
	float r = (float)luaL_checknumber(L, 5);
	float g = (float)luaL_checknumber(L, 6);
	float b = (float)luaL_checknumber(L, 7);
	float a = (float)luaL_optnumber(L, 8, 1.0);

	struct Color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_line(global_context->graphics, x1, y1, x2, y2);

	return 0;
}

static int lua_api_measure_text(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL)
	{
		return luaL_error(L, "Graphics context not available");
	}

	const char *text = luaL_checkstring(L, 1);
	int size = (int)luaL_checknumber(L, 2);

	float width = graphics_measure_text(global_context->graphics, text, size);
	lua_pushnumber(L, width);

	return 1;
}

static int lua_api_get_config(lua_State *L)
{
	const char *key = luaL_checkstring(L, 1);

	lua_getglobal(L, "config");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		return 1;
	}

	lua_getfield(L, -1, key);
	lua_remove(L, -2);

	return 1;
}

static int lua_api_register_command(lua_State *L)
{
	if (global_context == NULL || global_context->command_registry == NULL)
	{
		return luaL_error(L, "Command registry not available");
	}

	const char *command_name = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	if (!lua_command_registry_register_lua_command(
				global_context->command_registry, command_name, callback_ref
		))
	{
		luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);
		return luaL_error(L, "Failed to register command: %s", command_name);
	}

	return 0;
}

static enum key parse_key_string(const char *key_str)
{
	if (strcmp(key_str, "a") == 0)
		return KEY_A;
	if (strcmp(key_str, "b") == 0)
		return KEY_B;
	if (strcmp(key_str, "c") == 0)
		return KEY_C;
	if (strcmp(key_str, "d") == 0)
		return KEY_D;
	if (strcmp(key_str, "e") == 0)
		return KEY_E;
	if (strcmp(key_str, "f") == 0)
		return KEY_F;
	if (strcmp(key_str, "g") == 0)
		return KEY_G;
	if (strcmp(key_str, "h") == 0)
		return KEY_H;
	if (strcmp(key_str, "i") == 0)
		return KEY_I;
	if (strcmp(key_str, "j") == 0)
		return KEY_J;
	if (strcmp(key_str, "k") == 0)
		return KEY_K;
	if (strcmp(key_str, "l") == 0)
		return KEY_L;
	if (strcmp(key_str, "m") == 0)
		return KEY_M;
	if (strcmp(key_str, "n") == 0)
		return KEY_N;
	if (strcmp(key_str, "o") == 0)
		return KEY_O;
	if (strcmp(key_str, "p") == 0)
		return KEY_P;
	if (strcmp(key_str, "q") == 0)
		return KEY_Q;
	if (strcmp(key_str, "r") == 0)
		return KEY_R;
	if (strcmp(key_str, "s") == 0)
		return KEY_S;
	if (strcmp(key_str, "t") == 0)
		return KEY_T;
	if (strcmp(key_str, "u") == 0)
		return KEY_U;
	if (strcmp(key_str, "v") == 0)
		return KEY_V;
	if (strcmp(key_str, "w") == 0)
		return KEY_W;
	if (strcmp(key_str, "x") == 0)
		return KEY_X;
	if (strcmp(key_str, "y") == 0)
		return KEY_Y;
	if (strcmp(key_str, "z") == 0)
		return KEY_Z;
	if (strcmp(key_str, "space") == 0)
		return KEY_SPACE;
	if (strcmp(key_str, "escape") == 0)
		return KEY_ESCAPE;
	if (strcmp(key_str, "enter") == 0)
		return KEY_ENTER;
	if (strcmp(key_str, "tab") == 0)
		return KEY_TAB;
	if (strcmp(key_str, "f1") == 0)
		return KEY_F1;
	if (strcmp(key_str, "f2") == 0)
		return KEY_F2;
	if (strcmp(key_str, "f3") == 0)
		return KEY_F3;
	if (strcmp(key_str, "f4") == 0)
		return KEY_F4;
	if (strcmp(key_str, "f5") == 0)
		return KEY_F5;
	if (strcmp(key_str, "f6") == 0)
		return KEY_F6;
	if (strcmp(key_str, "f7") == 0)
		return KEY_F7;
	if (strcmp(key_str, "f8") == 0)
		return KEY_F8;
	if (strcmp(key_str, "f9") == 0)
		return KEY_F9;
	if (strcmp(key_str, "f10") == 0)
		return KEY_F10;
	if (strcmp(key_str, "f11") == 0)
		return KEY_F11;
	if (strcmp(key_str, "f12") == 0)
		return KEY_F12;
	if (strcmp(key_str, "up") == 0)
		return KEY_UP;
	if (strcmp(key_str, "down") == 0)
		return KEY_DOWN;
	if (strcmp(key_str, "left") == 0)
		return KEY_LEFT;
	if (strcmp(key_str, "right") == 0)
		return KEY_RIGHT;
	if (strcmp(key_str, "minus") == 0)
		return KEY_MINUS;
	if (strcmp(key_str, "equals") == 0)
		return KEY_EQUALS;

	return KEY_COUNT;
}

static int lua_api_register_keybinding(lua_State *L)
{
	if (global_context == NULL || global_context->command_registry == NULL)
	{
		return luaL_error(L, "Command registry not available");
	}

	const char *key_str = luaL_checkstring(L, 1);
	const char *command_name = luaL_checkstring(L, 2);

	bool shift = false;
	bool ctrl = false;
	bool alt = false;

	if (lua_istable(L, 3))
	{
		lua_getfield(L, 3, "shift");
		if (lua_isboolean(L, -1))
		{
			shift = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 3, "ctrl");
		if (lua_isboolean(L, -1))
		{
			ctrl = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 3, "alt");
		if (lua_isboolean(L, -1))
		{
			alt = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);
	}

	enum key key = parse_key_string(key_str);
	if (key == KEY_COUNT)
	{
		return luaL_error(L, "Invalid key: %s", key_str);
	}

	if (!lua_command_registry_register_keybinding(
				global_context->command_registry, key, shift, ctrl, alt, command_name
		))
	{
		return luaL_error(L, "Failed to register keybinding for: %s", key_str);
	}

	return 0;
}

static int lua_api_set_bpm(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL || global_context->audio == NULL)
	{
		return luaL_error(L, "App state or audio not available");
	}

	uint32_t bpm = (uint32_t)luaL_checknumber(L, 1);
	global_context->app_state->bpm = bpm;

	struct Sequencer* sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL)
	{
		sequencer_set_bpm(sequencer, bpm);
	}

	printf("BPM set to %u\n", bpm);

	return 0;
}

static int lua_api_set_playhead(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL || global_context->audio == NULL)
	{
		return luaL_error(L, "App state or audio not available");
	}

	uint32_t position_ms = (uint32_t)luaL_checknumber(L, 1);
	global_context->app_state->playhead_ms = position_ms;

	struct Sequencer* sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL)
	{
		sequencer_set_playhead(sequencer, position_ms);
	}

	printf("Playhead set to %u ms\n", position_ms);

	return 0;
}

static int lua_api_add_note(lua_State *L)
{
	if (global_context == NULL || global_context->audio == NULL)
	{
		return luaL_error(L, "Audio not available");
	}

	uint32_t start_ms = (uint32_t)luaL_checknumber(L, 1);
	uint8_t pitch = (uint8_t)luaL_checknumber(L, 2);
	uint32_t duration_ms = (uint32_t)luaL_checknumber(L, 3);

	struct NoteParams params = {
		.frequency = note_to_frequency(pitch),
		.duration_ms = (float)duration_ms,
		.waveform = WAVEFORM_SINE,
		.duty_cycle = 128,
		.decay = 0,
		.amplitude_dbfs = -3,
		.nes_noise_period = 15,
		.nes_noise_mode_flag = false,
		.voice_index = -1,
		.piano_key = pitch
	};

	if (lua_istable(L, 4)) {
		lua_getfield(L, 4, "waveform");
		if (!lua_isnil(L, -1)) {
			params.waveform = (enum WaveformType)lua_tointeger(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "duty_cycle");
		if (!lua_isnil(L, -1)) {
			params.duty_cycle = (uint8_t)lua_tointeger(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "decay");
		if (!lua_isnil(L, -1)) {
			params.decay = (int16_t)lua_tointeger(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "amplitude_dbfs");
		if (!lua_isnil(L, -1)) {
			params.amplitude_dbfs = (int8_t)lua_tointeger(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "nes_noise_period");
		if (!lua_isnil(L, -1)) {
			params.nes_noise_period = (uint8_t)lua_tointeger(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "nes_noise_mode_flag");
		if (!lua_isnil(L, -1)) {
			params.nes_noise_mode_flag = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);
	} else if (!lua_isnone(L, 4)) {
		params.waveform = (enum WaveformType)lua_tointeger(L, 4);
	}

	struct Sequencer* sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL)
	{
		sequencer_add_note(sequencer, start_ms, params);
	}

	printf("Note added: pitch=%d (%.2f Hz) at %ums for %ums waveform=%d\n",
		   pitch, params.frequency, start_ms, duration_ms, params.waveform);

	return 0;
}

static int lua_api_quit(lua_State *L)
{
	if (global_context == NULL || global_context->app_controller == NULL)
		return luaL_error(L, "App controller not available");

	void app_controller_stop(struct app_controller *);
	app_controller_stop(global_context->app_controller);
	printf("Quit requested\n");
	return 0;
}

static int lua_api_save(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Save requested\n");
	return 0;
}

static int lua_api_load(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Load requested\n");
	return 0;
}

static int lua_api_undo(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Undo requested\n");
	return 0;
}

static int lua_api_redo(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Redo requested\n");
	return 0;
}

static int lua_api_toggle_play(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL || global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = !global_context->app_state->playing;

	struct Sequencer* sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL)
	{
		if (global_context->app_state->playing)
		{
			sequencer_play(sequencer);
		}
		else
		{
			sequencer_pause(sequencer);
		}
	}

	printf("Playback %s\n", global_context->app_state->playing ? "started" : "stopped");
	return 0;
}

static int lua_api_play(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL || global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = true;

	struct Sequencer* sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL)
	{
		sequencer_play(sequencer);
	}

	printf("Playback started\n");
	return 0;
}

static int lua_api_pause(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL || global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = false;

	struct Sequencer* sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL)
	{
		sequencer_pause(sequencer);
	}

	printf("Playback paused\n");
	return 0;
}

static int lua_api_stop(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL || global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = false;
	global_context->app_state->playhead_ms = 0;

	struct Sequencer* sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL)
	{
		sequencer_stop(sequencer);
	}

	printf("Playback stopped\n");
	return 0;
}

static int lua_api_toggle_snap(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
		return luaL_error(L, "App state not available");
	global_context->app_state->snap_enabled = !global_context->app_state->snap_enabled;
	printf("Snap %s\n", global_context->app_state->snap_enabled ? "enabled" : "disabled");
	return 0;
}

static int lua_api_zoom_in(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Zoom in\n");
	return 0;
}

static int lua_api_zoom_out(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Zoom out\n");
	return 0;
}

static int lua_api_transpose_up(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Transpose up\n");
	return 0;
}

static int lua_api_transpose_down(lua_State *L)
{
	if (global_context == NULL)
		return luaL_error(L, "API context not available");
	printf("Transpose down\n");
	return 0;
}

static int lua_api_get_window_size(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
	{
		return luaL_error(L, "API context not available");
	}

	lua_pushinteger(L, global_context->app_state->window_width);
	lua_pushinteger(L, global_context->app_state->window_height);

	return 2;
}

static int lua_api_toggle_scale_highlight(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
	{
		return luaL_error(L, "API context not available");
	}

	global_context->app_state->show_scale_highlights =
			!global_context->app_state->show_scale_highlights;
	lua_pushboolean(L, global_context->app_state->show_scale_highlights);

	return 1;
}

static int lua_api_cycle_scale_type(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
	{
		return luaL_error(L, "API context not available");
	}

	int current = (int)global_context->app_state->selected_scale;
	int next = (current + 1) % SCALE_TYPE_COUNT;
	global_context->app_state->selected_scale = (enum scale_type)next;

	lua_pushstring(L, scale_type_to_string((enum scale_type)next));
	return 1;
}

static int lua_api_cycle_scale_root(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
	{
		return luaL_error(L, "API context not available");
	}

	int current = (int)global_context->app_state->selected_root;
	int next = (current + 1) % 12;
	global_context->app_state->selected_root = (enum root_note)next;

	lua_pushstring(L, root_note_to_string((enum root_note)next));
	return 1;
}

static int lua_api_get_scale_info(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
	{
		return luaL_error(L, "API context not available");
	}

	lua_newtable(L);

	lua_pushstring(L, scale_type_to_string(global_context->app_state->selected_scale));
	lua_setfield(L, -2, "scale");

	lua_pushstring(L, root_note_to_string(global_context->app_state->selected_root));
	lua_setfield(L, -2, "root");

	lua_pushboolean(L, global_context->app_state->show_scale_highlights);
	lua_setfield(L, -2, "highlight");

	return 1;
}

static int lua_api_toggle_fold(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
	{
		return luaL_error(L, "API context not available");
	}

	global_context->app_state->fold_mode = !global_context->app_state->fold_mode;

	if (global_context->app_state->fold_mode)
	{
		global_context->app_state->viewport.note_offset = 0;
	}

	lua_pushboolean(L, global_context->app_state->fold_mode);

	return 1;
}

static const char *waveform_type_to_string(enum WaveformType type)
{
	switch (type)
	{
	case WAVEFORM_SINE:
		return "sine";
	case WAVEFORM_SQUARE:
		return "square";
	case WAVEFORM_TRIANGLE:
		return "triangle";
	case WAVEFORM_SAWTOOTH:
		return "sawtooth";
	case WAVEFORM_NES_NOISE:
		return "nes_noise";
	default:
		return "unknown";
	}
}

static int lua_api_get_app_state(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL || global_context->audio == NULL)
	{
		return luaL_error(L, "API context not available");
	}

	struct app_state *state = global_context->app_state;
	struct Synth *synth = audio_get_synth(global_context->audio);

	lua_newtable(L);

	lua_pushinteger(L, state->bpm);
	lua_setfield(L, -2, "bpm");

	lua_pushinteger(L, state->selected_voice);
	lua_setfield(L, -2, "selected_voice");

	if (synth != NULL && state->selected_voice < MAX_VOICES)
	{
		enum WaveformType waveform = synth->voices[state->selected_voice].waveform;
		lua_pushstring(L, waveform_type_to_string(waveform));
		lua_setfield(L, -2, "waveform");
	}
	else
	{
		lua_pushstring(L, "square");
		lua_setfield(L, -2, "waveform");
	}

	lua_pushinteger(L, state->note_count);
	lua_setfield(L, -2, "note_count");

	lua_pushboolean(L, state->playing);
	lua_setfield(L, -2, "is_playing");

	lua_pushinteger(L, state->playhead_ms);
	lua_setfield(L, -2, "playhead_ms");

	lua_pushboolean(L, state->snap_enabled);
	lua_setfield(L, -2, "snap_enabled");

	lua_pushinteger(L, 50);
	lua_setfield(L, -2, "snap_ms");

	lua_pushstring(L, scale_type_to_string(state->selected_scale));
	lua_setfield(L, -2, "selected_scale");

	lua_pushstring(L, root_note_to_string(state->selected_root));
	lua_setfield(L, -2, "selected_root");

	return 1;
}

void lua_api_register_all(struct lua_runtime *runtime, struct lua_api_context *ctx)
{
	if (runtime == NULL || runtime->L == NULL)
	{
		return;
	}

	global_context = ctx;

	lua_newtable(runtime->L);

	lua_pushcfunction(runtime->L, lua_api_draw_rectangle);
	lua_setfield(runtime->L, -2, "drawRectangle");

	lua_pushcfunction(runtime->L, lua_api_draw_rectangle_outlined);
	lua_setfield(runtime->L, -2, "drawRectangleOutlined");

	lua_pushcfunction(runtime->L, lua_api_stroke_rectangle);
	lua_setfield(runtime->L, -2, "strokeRectangle");

	lua_pushcfunction(runtime->L, lua_api_draw_rounded_rectangle);
	lua_setfield(runtime->L, -2, "drawRoundedRectangle");

	lua_pushcfunction(runtime->L, lua_api_draw_rounded_rectangle_outlined);
	lua_setfield(runtime->L, -2, "drawRoundedRectangleOutlined");

	lua_pushcfunction(runtime->L, lua_api_stroke_rounded_rectangle);
	lua_setfield(runtime->L, -2, "strokeRoundedRectangle");

	lua_pushcfunction(runtime->L, lua_api_draw_line);
	lua_setfield(runtime->L, -2, "drawLine");

	lua_pushcfunction(runtime->L, lua_api_draw_text);
	lua_setfield(runtime->L, -2, "drawText");

	lua_pushcfunction(runtime->L, lua_api_measure_text);
	lua_setfield(runtime->L, -2, "measureText");

	lua_pushcfunction(runtime->L, lua_api_get_config);
	lua_setfield(runtime->L, -2, "getConfig");

	lua_pushcfunction(runtime->L, lua_api_register_command);
	lua_setfield(runtime->L, -2, "registerCommand");

	lua_pushcfunction(runtime->L, lua_api_register_keybinding);
	lua_setfield(runtime->L, -2, "registerKeybinding");

	lua_pushcfunction(runtime->L, lua_api_set_bpm);
	lua_setfield(runtime->L, -2, "setBpm");

	lua_pushcfunction(runtime->L, lua_api_set_playhead);
	lua_setfield(runtime->L, -2, "setPlayhead");

	lua_pushcfunction(runtime->L, lua_api_add_note);
	lua_setfield(runtime->L, -2, "addNote");

	lua_pushcfunction(runtime->L, lua_api_quit);
	lua_setfield(runtime->L, -2, "quit");

	lua_pushcfunction(runtime->L, lua_api_save);
	lua_setfield(runtime->L, -2, "save");

	lua_pushcfunction(runtime->L, lua_api_load);
	lua_setfield(runtime->L, -2, "load");

	lua_pushcfunction(runtime->L, lua_api_undo);
	lua_setfield(runtime->L, -2, "undo");

	lua_pushcfunction(runtime->L, lua_api_redo);
	lua_setfield(runtime->L, -2, "redo");

	lua_pushcfunction(runtime->L, lua_api_toggle_play);
	lua_setfield(runtime->L, -2, "togglePlay");

	lua_pushcfunction(runtime->L, lua_api_play);
	lua_setfield(runtime->L, -2, "play");

	lua_pushcfunction(runtime->L, lua_api_pause);
	lua_setfield(runtime->L, -2, "pause");

	lua_pushcfunction(runtime->L, lua_api_stop);
	lua_setfield(runtime->L, -2, "stop");

	lua_pushcfunction(runtime->L, lua_api_toggle_snap);
	lua_setfield(runtime->L, -2, "toggleSnap");

	lua_pushcfunction(runtime->L, lua_api_zoom_in);
	lua_setfield(runtime->L, -2, "zoomIn");

	lua_pushcfunction(runtime->L, lua_api_zoom_out);
	lua_setfield(runtime->L, -2, "zoomOut");

	lua_pushcfunction(runtime->L, lua_api_transpose_up);
	lua_setfield(runtime->L, -2, "transposeUp");

	lua_pushcfunction(runtime->L, lua_api_transpose_down);
	lua_setfield(runtime->L, -2, "transposeDown");

	lua_pushcfunction(runtime->L, lua_api_get_window_size);
	lua_setfield(runtime->L, -2, "getWindowSize");

	lua_pushcfunction(runtime->L, lua_api_toggle_scale_highlight);
	lua_setfield(runtime->L, -2, "toggleScaleHighlight");

	lua_pushcfunction(runtime->L, lua_api_cycle_scale_type);
	lua_setfield(runtime->L, -2, "cycleScaleType");

	lua_pushcfunction(runtime->L, lua_api_cycle_scale_root);
	lua_setfield(runtime->L, -2, "cycleScaleRoot");

	lua_pushcfunction(runtime->L, lua_api_get_scale_info);
	lua_setfield(runtime->L, -2, "getScaleInfo");

	lua_pushcfunction(runtime->L, lua_api_toggle_fold);
	lua_setfield(runtime->L, -2, "toggleFold");

	lua_pushcfunction(runtime->L, lua_api_get_app_state);
	lua_setfield(runtime->L, -2, "getAppState");

	lua_pushinteger(runtime->L, WAVEFORM_SINE);
	lua_setfield(runtime->L, -2, "WAVEFORM_SINE");

	lua_pushinteger(runtime->L, WAVEFORM_SQUARE);
	lua_setfield(runtime->L, -2, "WAVEFORM_SQUARE");

	lua_pushinteger(runtime->L, WAVEFORM_TRIANGLE);
	lua_setfield(runtime->L, -2, "WAVEFORM_TRIANGLE");

	lua_pushinteger(runtime->L, WAVEFORM_SAWTOOTH);
	lua_setfield(runtime->L, -2, "WAVEFORM_SAWTOOTH");

	lua_pushinteger(runtime->L, WAVEFORM_NES_NOISE);
	lua_setfield(runtime->L, -2, "WAVEFORM_NES_NOISE");

	lua_setglobal(runtime->L, "boostio");
}

void lua_api_set_context(struct lua_api_context *ctx)
{
	global_context = ctx;
}
