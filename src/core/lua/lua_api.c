#include "core/lua/lua_api.h"
#include "app/app_state.h"
#include "app/lua_command_registry.h"
#include "app/viewport_utils.h"
#include "core/audio/audio.h"
#include "core/audio/c_exporter.h"
#include "core/audio/scale.h"
#include "core/audio/sequencer.h"
#include "core/audio/song_loader.h"
#include "core/audio/song_saver.h"
#include "core/audio/synth.h"
#include "core/audio/wav_exporter.h"
#include "core/graphics/color.h"
#include "core/graphics/graphics.h"
#include "core/graphics/window.h"
#include "core/input/input_types.h"
#include "core/platform/path_utils.h"

#include <SDL3/SDL.h>
#include <string.h>

static struct lua_api_context *global_context = NULL;

static int lua_api_draw_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rect(global_context->graphics, x, y, width, height);

	return 0;
}

static int lua_api_draw_text(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_text(global_context->graphics, text, x, y, size);

	return 0;
}

static int lua_api_draw_rounded_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rounded_rect(global_context->graphics, x, y, width, height, radius);

	return 0;
}

static int lua_api_draw_rectangle_outlined(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	struct color outline_color = color_from_floats(or, og, ob, oa);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rect_outlined(
		global_context->graphics, x, y, width, height, outline_color, outline_width
	);

	return 0;
}

static int lua_api_stroke_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_rect(global_context->graphics, x, y, width, height);

	return 0;
}

static int lua_api_draw_rounded_rectangle_outlined(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	struct color outline_color = color_from_floats(or, og, ob, oa);
	graphics_set_color(global_context->graphics, color);
	graphics_fill_rounded_rect_outlined(
		global_context->graphics, x, y, width, height, radius, outline_color, outline_width
	);

	return 0;
}

static int lua_api_stroke_rounded_rectangle(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_rounded_rect(global_context->graphics, x, y, width, height, radius);

	return 0;
}

static int lua_api_draw_line(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
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

	struct color color = color_from_floats(r, g, b, a);
	graphics_set_color(global_context->graphics, color);
	graphics_draw_line(global_context->graphics, x1, y1, x2, y2);

	return 0;
}

static int lua_api_measure_text(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
		return luaL_error(L, "Graphics context not available");
	}

	const char *text = luaL_checkstring(L, 1);
	int size = (int)luaL_checknumber(L, 2);

	float width = graphics_measure_text(global_context->graphics, text, size);
	lua_pushnumber(L, width);

	return 1;
}

static int lua_api_hex_to_rgb(lua_State *L)
{
	const char *hex = luaL_checkstring(L, 1);

	if (hex[0] == '#') {
		hex++;
	}

	unsigned int r = 0, g = 0, b = 0;
	if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) != 3) {
		return luaL_error(L, "Invalid hex color format");
	}

	lua_newtable(L);
	lua_pushnumber(L, r / 255.0);
	lua_setfield(L, -2, "r");
	lua_pushnumber(L, g / 255.0);
	lua_setfield(L, -2, "g");
	lua_pushnumber(L, b / 255.0);
	lua_setfield(L, -2, "b");

	return 1;
}

static int lua_api_is_point_in_rect(lua_State *L)
{
	double px = luaL_checknumber(L, 1);
	double py = luaL_checknumber(L, 2);
	double x = luaL_checknumber(L, 3);
	double y = luaL_checknumber(L, 4);
	double width = luaL_checknumber(L, 5);
	double height = luaL_checknumber(L, 6);

	bool inside = px >= x && px <= x + width && py >= y && py <= y + height;
	lua_pushboolean(L, inside);

	return 1;
}

static int lua_api_get_config(lua_State *L)
{
	const char *key = luaL_checkstring(L, 1);

	lua_getglobal(L, "config");
	if (!lua_istable(L, -1)) {
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
	if (global_context == NULL || global_context->command_registry == NULL) {
		return luaL_error(L, "Command registry not available");
	}

	const char *command_name = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	if (!lua_command_registry_register_lua_command(
		    global_context->command_registry, command_name, callback_ref
	    )) {
		luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);
		return luaL_error(L, "Failed to register command: %s", command_name);
	}

	return 0;
}

static enum key parse_key_string(const char *key_str)
{
	if (strcmp(key_str, "0") == 0)
		return KEY_0;
	if (strcmp(key_str, "1") == 0)
		return KEY_1;
	if (strcmp(key_str, "2") == 0)
		return KEY_2;
	if (strcmp(key_str, "3") == 0)
		return KEY_3;
	if (strcmp(key_str, "4") == 0)
		return KEY_4;
	if (strcmp(key_str, "5") == 0)
		return KEY_5;
	if (strcmp(key_str, "6") == 0)
		return KEY_6;
	if (strcmp(key_str, "7") == 0)
		return KEY_7;
	if (strcmp(key_str, "8") == 0)
		return KEY_8;
	if (strcmp(key_str, "9") == 0)
		return KEY_9;
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
	if (strcmp(key_str, "shift") == 0)
		return KEY_LEFT_SHIFT;
	if (strcmp(key_str, "ctrl") == 0)
		return KEY_LEFT_CONTROL;
	if (strcmp(key_str, "alt") == 0)
		return KEY_LEFT_ALT;
	if (strcmp(key_str, "delete") == 0)
		return KEY_DELETE;
	if (strcmp(key_str, "backspace") == 0)
		return KEY_BACKSPACE;

	return KEY_COUNT;
}

static int lua_api_register_keybinding(lua_State *L)
{
	if (global_context == NULL || global_context->command_registry == NULL) {
		return luaL_error(L, "Command registry not available");
	}

	const char *key_str = luaL_checkstring(L, 1);
	const char *command_name = luaL_checkstring(L, 2);

	bool shift = false;
	bool ctrl = false;
	bool alt = false;

	if (lua_istable(L, 3)) {
		lua_getfield(L, 3, "shift");
		if (lua_isboolean(L, -1)) {
			shift = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 3, "ctrl");
		if (lua_isboolean(L, -1)) {
			ctrl = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 3, "alt");
		if (lua_isboolean(L, -1)) {
			alt = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);
	}

	enum key key = parse_key_string(key_str);
	if (key == KEY_COUNT) {
		return luaL_error(L, "Invalid key: %s", key_str);
	}

	if (!lua_command_registry_register_keybinding(
		    global_context->command_registry, key, shift, ctrl, alt, command_name
	    )) {
		return luaL_error(L, "Failed to register keybinding for: %s", key_str);
	}

	return 0;
}

static int lua_api_set_bpm(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "App state or audio not available");
	}

	uint32_t bpm = (uint32_t)luaL_checknumber(L, 1);
	global_context->app_state->bpm = bpm;

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		sequencer_set_bpm(sequencer, bpm);
	}

	printf("BPM set to %u\n", bpm);

	return 0;
}

static int lua_api_set_playhead(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "App state or audio not available");
	}

	uint32_t position_ms = (uint32_t)luaL_checknumber(L, 1);
	global_context->app_state->playhead_ms = position_ms;

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		sequencer_set_playhead(sequencer, position_ms);
	}

	printf("Playhead set to %u ms\n", position_ms);

	return 0;
}

static int lua_api_add_note(lua_State *L)
{
	if (global_context == NULL || global_context->audio == NULL ||
	    global_context->app_state == NULL) {
		return luaL_error(L, "Audio not available");
	}

	uint32_t start_ms = (uint32_t)luaL_checknumber(L, 1);
	uint8_t pitch = (uint8_t)luaL_checknumber(L, 2);
	uint32_t duration_ms = (uint32_t)luaL_checknumber(L, 3);

	struct app_state *state = global_context->app_state;
	uint8_t voice = state->selected_voice;

	if (state->selected_instrument >= state->instrument_count) {
		return luaL_error(L, "Invalid instrument selected");
	}

	struct instrument *instr = &state->instruments[state->selected_instrument];

	struct note_params params = {
		.frequency = note_to_frequency(pitch),
		.duration_ms = (float)duration_ms,
		.waveform = instr->waveform,
		.duty_cycle = instr->duty_cycle,
		.decay = instr->decay,
		.amplitude_dbfs = instr->amplitude_dbfs,
		.nes_noise_period = 15,
		.nes_noise_mode_flag = instr->nes_noise_mode_flag,
		.nes_noise_lfsr_init = instr->nes_noise_lfsr,
		.restart_phase = true,
		.voice_index = voice,
		.piano_key = pitch
	};

	if (lua_istable(L, 4)) {
		lua_getfield(L, 4, "voice");
		if (!lua_isnil(L, -1)) {
			voice = (uint8_t)lua_tointeger(L, -1);
			params.voice_index = voice;
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "waveform");
		if (!lua_isnil(L, -1)) {
			params.waveform = (enum waveform_type)lua_tointeger(L, -1);
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
		params.waveform = (enum waveform_type)lua_tointeger(L, 4);
	}

	if (state->note_count < UI_MAX_NOTES) {
		struct ui_note *ui_note = &state->notes[state->note_count];
		ui_note->id = state->next_note_id++;
		ui_note->ms = start_ms;
		ui_note->duration_ms = (uint16_t)duration_ms;
		ui_note->voice = voice;
		ui_note->piano_key = pitch;
		ui_note->waveform = params.waveform;
		ui_note->duty_cycle = params.duty_cycle;
		ui_note->decay = params.decay;
		ui_note->amplitude_dbfs = params.amplitude_dbfs;
		ui_note->nes_noise_period = params.nes_noise_period;
		ui_note->nes_noise_mode_flag = params.nes_noise_mode_flag;
		ui_note->nes_noise_lfsr_init = params.nes_noise_lfsr_init;
		ui_note->restart_phase = params.restart_phase;
		state->note_count++;

		struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
		if (sequencer != NULL) {
			app_state_sync_notes_to_sequencer(state, sequencer, global_context->audio);
		}

		lua_pushinteger(L, ui_note->id);
		return 1;
	}

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
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL)
		return luaL_error(L, "API context not available");

	const char *filepath = luaL_optstring(L, 1, NULL);

	char base_path[512];
	if (filepath != NULL) {
		strncpy(base_path, filepath, 511);
		base_path[511] = '\0';
		strncpy(global_context->app_state->current_file_path, filepath, 511);
		global_context->app_state->current_file_path[511] = '\0';
	} else if (global_context->app_state->current_file_path[0] != '\0') {
		strncpy(base_path, global_context->app_state->current_file_path, 511);
		base_path[511] = '\0';
	} else {
		strncpy(base_path, "song.json", 511);
		base_path[511] = '\0';
	}

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer == NULL)
		return luaL_error(L, "Sequencer not available");

	char json_path[512];
	char c_path[512];
	char wav_path[512];

	path_build_with_extension(base_path, ".json", json_path, sizeof(json_path));
	path_build_with_extension(base_path, ".c", c_path, sizeof(c_path));
	path_build_with_extension(base_path, ".wav", wav_path, sizeof(wav_path));

	bool success_json =
		song_saver_save_to_file(global_context->app_state, sequencer, json_path);

	bool success_c = c_exporter_export_to_file(sequencer, c_path);
	bool success_wav = wav_exporter_export_to_file(sequencer, wav_path);

	bool success = success_json && success_c && success_wav;

	if (success) {
		printf("Saved all formats successfully:\n");
		printf("  - %s\n", json_path);
		printf("  - %s\n", c_path);
		printf("  - %s\n", wav_path);
	}

	lua_pushboolean(L, success);
	return 1;
}

static int lua_api_save_c(lua_State *L)
{
	if (global_context == NULL || global_context->audio == NULL)
		return luaL_error(L, "API context not available");

	const char *filepath = luaL_optstring(L, 1, "song.c");

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer == NULL)
		return luaL_error(L, "Sequencer not available");

	bool success = c_exporter_export_to_file(sequencer, filepath);

	lua_pushboolean(L, success);
	return 1;
}

static int lua_api_save_wav(lua_State *L)
{
	if (global_context == NULL || global_context->audio == NULL)
		return luaL_error(L, "API context not available");

	const char *filepath = luaL_optstring(L, 1, "song.wav");

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer == NULL)
		return luaL_error(L, "Sequencer not available");

	bool success = wav_exporter_export_to_file(sequencer, filepath);

	lua_pushboolean(L, success);
	return 1;
}

static int lua_api_load(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL)
		return luaL_error(L, "API context not available");

	const char *filepath = luaL_optstring(L, 1, "song.json");

	struct audio *audio = global_context->audio;
	bool success = song_loader_load_from_file(audio, global_context->app_state, filepath);

	if (success && global_context->app_state) {
		strncpy(global_context->app_state->current_file_path, filepath, 511);
		global_context->app_state->current_file_path[511] = '\0';
		printf("Loaded and set current file path: %s\n", filepath);
	}

	lua_pushboolean(L, success);
	return 1;
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
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = !global_context->app_state->playing;

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		if (global_context->app_state->playing) {
			sequencer_play(sequencer);
		} else {
			sequencer_pause(sequencer);
		}
	}

	return 0;
}

static int lua_api_play(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = true;

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		sequencer_play(sequencer);
	}

	return 0;
}

static int lua_api_pause(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = false;

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		sequencer_pause(sequencer);
	}

	return 0;
}

static int lua_api_stop(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL)
		return luaL_error(L, "App state or audio not available");

	global_context->app_state->playing = false;
	global_context->app_state->playhead_ms = 0;

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		sequencer_stop(sequencer);
	}

	return 0;
}

static int lua_api_toggle_snap(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL)
		return luaL_error(L, "App state not available");
	global_context->app_state->snap_enabled = !global_context->app_state->snap_enabled;
	lua_pushboolean(L, global_context->app_state->snap_enabled);
	return 1;
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
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	lua_pushinteger(L, global_context->app_state->window_width);
	lua_pushinteger(L, global_context->app_state->window_height);

	return 2;
}

static int lua_api_get_fps(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
		return luaL_error(L, "Graphics context not available");
	}

	float fps = graphics_get_fps(global_context->graphics);
	lua_pushnumber(L, fps);

	return 1;
}

static int lua_api_toggle_scale_highlight(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	global_context->app_state->show_scale_highlights =
		!global_context->app_state->show_scale_highlights;
	lua_pushboolean(L, global_context->app_state->show_scale_highlights);

	return 1;
}

static int lua_api_cycle_scale_type(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
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
	if (global_context == NULL || global_context->app_state == NULL) {
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
	if (global_context == NULL || global_context->app_state == NULL) {
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

static int lua_api_get_mouse_position(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
		return luaL_error(L, "API context not available");
	}

	struct window *window = graphics_get_window(global_context->graphics);
	if (window == NULL) {
		return luaL_error(L, "Window not available");
	}

	int x, y;
	window_get_mouse_position(window, &x, &y);

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	return 2;
}

static int lua_api_is_mouse_button_down(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
		return luaL_error(L, "API context not available");
	}

	int button = (int)luaL_checkinteger(L, 1);

	struct window *window = graphics_get_window(global_context->graphics);
	if (window == NULL) {
		return luaL_error(L, "Window not available");
	}

	bool is_down = window_is_mouse_button_down(window, button - 1);
	lua_pushboolean(L, is_down);

	return 1;
}

static int lua_api_is_mouse_button_pressed(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
		return luaL_error(L, "API context not available");
	}

	int button = (int)luaL_checkinteger(L, 1);

	struct window *window = graphics_get_window(global_context->graphics);
	if (window == NULL) {
		return luaL_error(L, "Window not available");
	}

	bool is_pressed = window_is_mouse_button_pressed(window, button - 1);
	lua_pushboolean(L, is_pressed);

	return 1;
}

static int lua_api_toggle_fold(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	global_context->app_state->fold_mode = !global_context->app_state->fold_mode;

	if (global_context->app_state->fold_mode) {
		global_context->app_state->viewport.note_offset = 0;
	}

	lua_pushboolean(L, global_context->app_state->fold_mode);

	return 1;
}

static int lua_api_set_voice_hidden(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	int voice = (int)luaL_checkinteger(L, 1);
	bool hidden = lua_toboolean(L, 2);

	if (voice < 0 || voice >= 8) {
		return luaL_error(L, "Voice must be 0-7");
	}

	global_context->app_state->voice_hidden[voice] = hidden;

	return 0;
}

static int lua_api_set_voice_solo(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	int voice = (int)luaL_checkinteger(L, 1);
	bool solo = lua_toboolean(L, 2);

	if (voice < 0 || voice >= 8) {
		return luaL_error(L, "Voice must be 0-7");
	}

	global_context->app_state->voice_solo[voice] = solo;

	return 0;
}

static int lua_api_set_voice_muted(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	int voice = (int)luaL_checkinteger(L, 1);
	bool muted = lua_toboolean(L, 2);

	if (voice < 0 || voice >= 8) {
		return luaL_error(L, "Voice must be 0-7");
	}

	global_context->app_state->voice_muted[voice] = muted;

	return 0;
}

static int lua_api_set_selected_voice(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	int voice = (int)luaL_checkinteger(L, 1);

	if (voice < 0 || voice >= 8) {
		return luaL_error(L, "Voice must be 0-7");
	}

	global_context->app_state->selected_voice = (uint8_t)voice;

	return 0;
}

static int lua_api_set_note_voice(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t note_id = (uint32_t)luaL_checkinteger(L, 1);
	int voice = (int)luaL_checkinteger(L, 2);

	if (voice < 0 || voice >= 8) {
		return luaL_error(L, "Voice must be 0-7");
	}

	struct app_state *state = global_context->app_state;

	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == note_id) {
			state->notes[i].voice = (uint8_t)voice;
			break;
		}
	}

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		app_state_sync_notes_to_sequencer(state, sequencer, global_context->audio);
	}

	return 0;
}

static const char *waveform_type_to_string(enum waveform_type type)
{
	switch (type) {
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
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "API context not available");
	}

	struct app_state *state = global_context->app_state;
	struct synth *synth = audio_get_synth(global_context->audio);

	lua_newtable(L);

	lua_pushinteger(L, state->bpm);
	lua_setfield(L, -2, "bpm");

	lua_pushinteger(L, state->selected_voice);
	lua_setfield(L, -2, "selected_voice");

	lua_pushinteger(L, state->selected_instrument);
	lua_setfield(L, -2, "selected_instrument");

	if (synth != NULL && state->selected_voice < MAX_VOICES) {
		enum waveform_type waveform = synth->voices[state->selected_voice].waveform;
		lua_pushstring(L, waveform_type_to_string(waveform));
		lua_setfield(L, -2, "waveform");
	} else {
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

	lua_newtable(L);
	for (int i = 0; i < 8; i++) {
		lua_pushboolean(L, state->voice_hidden[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_setfield(L, -2, "voice_hidden");

	lua_newtable(L);
	for (int i = 0; i < 8; i++) {
		lua_pushboolean(L, state->voice_solo[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_setfield(L, -2, "voice_solo");

	lua_newtable(L);
	for (int i = 0; i < 8; i++) {
		lua_pushboolean(L, state->voice_muted[i]);
		lua_rawseti(L, -2, i + 1);
	}
	lua_setfield(L, -2, "voice_muted");

	lua_newtable(L);
	lua_pushnumber(L, state->viewport.time_offset);
	lua_setfield(L, -2, "time_offset");
	lua_pushinteger(L, state->viewport.note_offset);
	lua_setfield(L, -2, "note_offset");
	lua_pushnumber(L, state->viewport.pixels_per_ms);
	lua_setfield(L, -2, "pixels_per_ms");
	lua_pushnumber(L, state->viewport.piano_key_height);
	lua_setfield(L, -2, "piano_key_height");
	lua_pushnumber(L, state->viewport.grid_x);
	lua_setfield(L, -2, "grid_x");
	lua_pushnumber(L, state->viewport.grid_y);
	lua_setfield(L, -2, "grid_y");
	lua_pushnumber(L, state->viewport.grid_width);
	lua_setfield(L, -2, "grid_width");
	lua_pushnumber(L, state->viewport.grid_height);
	lua_setfield(L, -2, "grid_height");
	lua_setfield(L, -2, "viewport");

	lua_newtable(L);
	for (uint32_t i = 0; i < state->note_count; i++) {
		lua_newtable(L);
		lua_pushinteger(L, state->notes[i].id);
		lua_setfield(L, -2, "id");
		lua_pushinteger(L, state->notes[i].ms);
		lua_setfield(L, -2, "ms");
		lua_pushinteger(L, state->notes[i].duration_ms);
		lua_setfield(L, -2, "duration_ms");
		lua_pushinteger(L, state->notes[i].voice);
		lua_setfield(L, -2, "voice");
		lua_pushinteger(L, state->notes[i].piano_key);
		lua_setfield(L, -2, "piano_key");
		lua_pushinteger(L, state->notes[i].waveform);
		lua_setfield(L, -2, "waveform");
		lua_pushinteger(L, state->notes[i].duty_cycle);
		lua_setfield(L, -2, "duty_cycle");
		lua_pushinteger(L, state->notes[i].decay);
		lua_setfield(L, -2, "decay");
		lua_pushinteger(L, state->notes[i].amplitude_dbfs);
		lua_setfield(L, -2, "amplitude_dbfs");
		lua_pushinteger(L, state->notes[i].nes_noise_period);
		lua_setfield(L, -2, "nes_noise_period");
		lua_pushboolean(L, state->notes[i].nes_noise_mode_flag);
		lua_setfield(L, -2, "nes_noise_mode_flag");
		lua_pushinteger(L, state->notes[i].nes_noise_lfsr_init);
		lua_setfield(L, -2, "nes_noise_lfsr_init");
		lua_pushboolean(L, state->notes[i].restart_phase);
		lua_setfield(L, -2, "restart_phase");
		lua_rawseti(L, -2, i + 1);
	}
	lua_setfield(L, -2, "notes");

	lua_pushboolean(L, state->fold_mode);
	lua_setfield(L, -2, "fold_mode");

	lua_pushboolean(L, state->show_scale_highlights);
	lua_setfield(L, -2, "show_scale_highlights");

	return 1;
}

static int lua_api_get_instrument_count(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	lua_pushinteger(L, global_context->app_state->instrument_count);
	return 1;
}

static int lua_api_get_instrument(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	int index = (int)luaL_checkinteger(L, 1);

	if (index < 0 || index >= global_context->app_state->instrument_count) {
		return luaL_error(L, "Instrument index out of range");
	}

	struct instrument *inst = &global_context->app_state->instruments[index];

	lua_newtable(L);

	lua_pushstring(L, inst->name);
	lua_setfield(L, -2, "name");

	lua_pushinteger(L, inst->waveform);
	lua_setfield(L, -2, "waveform");

	lua_pushinteger(L, inst->duty_cycle);
	lua_setfield(L, -2, "duty_cycle");

	lua_pushinteger(L, inst->amplitude_dbfs);
	lua_setfield(L, -2, "amplitude_dbfs");

	lua_pushinteger(L, inst->decay);
	lua_setfield(L, -2, "decay");

	lua_pushinteger(L, inst->default_duration_ms);
	lua_setfield(L, -2, "default_duration_ms");

	lua_pushinteger(L, inst->color_r);
	lua_setfield(L, -2, "color_r");

	lua_pushinteger(L, inst->color_g);
	lua_setfield(L, -2, "color_g");

	lua_pushinteger(L, inst->color_b);
	lua_setfield(L, -2, "color_b");

	lua_pushboolean(L, inst->nes_noise_mode_flag);
	lua_setfield(L, -2, "nes_noise_mode_flag");

	lua_pushinteger(L, inst->nes_noise_lfsr);
	lua_setfield(L, -2, "nes_noise_lfsr");

	return 1;
}

static int lua_api_set_selected_instrument(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	int index = (int)luaL_checkinteger(L, 1);

	if (index < 0 || index >= global_context->app_state->instrument_count) {
		return luaL_error(L, "Instrument index out of range");
	}

	global_context->app_state->selected_instrument = (uint8_t)index;

	return 0;
}

static enum scale_type parse_scale_type_string(const char *scale_str)
{
	if (strcmp(scale_str, "chromatic") == 0)
		return SCALE_CHROMATIC;
	if (strcmp(scale_str, "major") == 0)
		return SCALE_MAJOR;
	if (strcmp(scale_str, "minor") == 0)
		return SCALE_MINOR;
	if (strcmp(scale_str, "harmonic_minor") == 0)
		return SCALE_HARMONIC_MINOR;
	if (strcmp(scale_str, "melodic_minor") == 0)
		return SCALE_MELODIC_MINOR;
	if (strcmp(scale_str, "pentatonic_major") == 0)
		return SCALE_PENTATONIC_MAJOR;
	if (strcmp(scale_str, "pentatonic_minor") == 0)
		return SCALE_PENTATONIC_MINOR;
	if (strcmp(scale_str, "blues") == 0)
		return SCALE_BLUES;
	if (strcmp(scale_str, "dorian") == 0)
		return SCALE_DORIAN;
	if (strcmp(scale_str, "phrygian") == 0)
		return SCALE_PHRYGIAN;
	if (strcmp(scale_str, "lydian") == 0)
		return SCALE_LYDIAN;
	if (strcmp(scale_str, "mixolydian") == 0)
		return SCALE_MIXOLYDIAN;
	if (strcmp(scale_str, "aeolian") == 0)
		return SCALE_AEOLIAN;
	if (strcmp(scale_str, "locrian") == 0)
		return SCALE_LOCRIAN;
	return SCALE_CHROMATIC;
}

static enum root_note parse_root_note_string(const char *root_str)
{
	if (strcmp(root_str, "C") == 0)
		return ROOT_C;
	if (strcmp(root_str, "C#") == 0)
		return ROOT_CS;
	if (strcmp(root_str, "D") == 0)
		return ROOT_D;
	if (strcmp(root_str, "D#") == 0)
		return ROOT_DS;
	if (strcmp(root_str, "E") == 0)
		return ROOT_E;
	if (strcmp(root_str, "F") == 0)
		return ROOT_F;
	if (strcmp(root_str, "F#") == 0)
		return ROOT_FS;
	if (strcmp(root_str, "G") == 0)
		return ROOT_G;
	if (strcmp(root_str, "G#") == 0)
		return ROOT_GS;
	if (strcmp(root_str, "A") == 0)
		return ROOT_A;
	if (strcmp(root_str, "A#") == 0)
		return ROOT_AS;
	if (strcmp(root_str, "B") == 0)
		return ROOT_B;
	return ROOT_C;
}

static int lua_api_is_note_in_scale(lua_State *L)
{
	uint8_t piano_key = (uint8_t)luaL_checkinteger(L, 1);
	const char *scale_str = luaL_checkstring(L, 2);
	const char *root_str = luaL_checkstring(L, 3);

	enum scale_type scale = parse_scale_type_string(scale_str);
	enum root_note root = parse_root_note_string(root_str);

	bool result = scale_is_note_in_scale(piano_key, scale, root);
	lua_pushboolean(L, result);

	return 1;
}

static int lua_api_is_root_note(lua_State *L)
{
	uint8_t piano_key = (uint8_t)luaL_checkinteger(L, 1);
	const char *root_str = luaL_checkstring(L, 2);

	enum root_note root = parse_root_note_string(root_str);

	bool result = scale_is_root_note(piano_key, root);
	lua_pushboolean(L, result);

	return 1;
}

static int lua_api_ms_to_x(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t ms = (uint32_t)luaL_checknumber(L, 1);
	float x;

	viewport_ms_to_x(&global_context->app_state->viewport, ms, &x);
	lua_pushnumber(L, x);

	return 1;
}

static int lua_api_x_to_ms(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	float x = (float)luaL_checknumber(L, 1);
	uint32_t ms;

	viewport_x_to_ms(&global_context->app_state->viewport, x, &ms);
	lua_pushinteger(L, ms);

	return 1;
}

static int lua_api_piano_key_to_y(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint8_t piano_key = (uint8_t)luaL_checkinteger(L, 1);
	float y;

	viewport_piano_key_to_y(&global_context->app_state->viewport, piano_key, &y);
	lua_pushnumber(L, y);

	return 1;
}

static int lua_api_y_to_piano_key(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	float y = (float)luaL_checknumber(L, 1);
	uint8_t piano_key;

	viewport_y_to_piano_key(&global_context->app_state->viewport, y, &piano_key);
	lua_pushinteger(L, piano_key);

	return 1;
}

static int lua_api_get_selection(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	struct app_state *state = global_context->app_state;

	lua_newtable(L);
	for (uint32_t i = 0; i < state->selection.count; i++) {
		lua_pushinteger(L, state->selection.selected_ids[i]);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

static int lua_api_clear_selection(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	global_context->app_state->selection.count = 0;

	return 0;
}

static int lua_api_select_note(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t note_id = (uint32_t)luaL_checkinteger(L, 1);
	struct app_state *state = global_context->app_state;

	bool already_selected = false;
	for (uint32_t i = 0; i < state->selection.count; i++) {
		if (state->selection.selected_ids[i] == note_id) {
			already_selected = true;
			break;
		}
	}

	if (!already_selected && state->selection.count < UI_MAX_SELECTION) {
		state->selection.selected_ids[state->selection.count] = note_id;
		state->selection.count++;
	}

	return 0;
}

static int lua_api_deselect_note(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t note_id = (uint32_t)luaL_checkinteger(L, 1);
	struct app_state *state = global_context->app_state;

	for (uint32_t i = 0; i < state->selection.count; i++) {
		if (state->selection.selected_ids[i] == note_id) {
			for (uint32_t j = i; j < state->selection.count - 1; j++) {
				state->selection.selected_ids[j] =
					state->selection.selected_ids[j + 1];
			}
			state->selection.count--;
			break;
		}
	}

	return 0;
}

static int lua_api_is_note_selected(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t note_id = (uint32_t)luaL_checkinteger(L, 1);
	struct app_state *state = global_context->app_state;

	bool is_selected = false;
	for (uint32_t i = 0; i < state->selection.count; i++) {
		if (state->selection.selected_ids[i] == note_id) {
			is_selected = true;
			break;
		}
	}

	lua_pushboolean(L, is_selected);

	return 1;
}

static int lua_api_move_note(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t note_id = (uint32_t)luaL_checkinteger(L, 1);
	int32_t delta_ms = (int32_t)luaL_checkinteger(L, 2);
	int32_t delta_piano_key = (int32_t)luaL_checkinteger(L, 3);

	struct app_state *state = global_context->app_state;

	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == note_id) {
			int32_t new_ms = (int32_t)state->notes[i].ms + delta_ms;
			if (new_ms < 0)
				new_ms = 0;
			state->notes[i].ms = (uint32_t)new_ms;

			int32_t new_key = (int32_t)state->notes[i].piano_key + delta_piano_key;
			if (new_key < 0)
				new_key = 0;
			if (new_key > 127)
				new_key = 127;
			state->notes[i].piano_key = (uint8_t)new_key;
			break;
		}
	}

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		app_state_sync_notes_to_sequencer(state, sequencer, global_context->audio);
	}

	return 0;
}

static int lua_api_resize_note(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t note_id = (uint32_t)luaL_checkinteger(L, 1);
	int32_t delta_duration_ms = (int32_t)luaL_checkinteger(L, 2);
	bool from_left = lua_toboolean(L, 3);

	struct app_state *state = global_context->app_state;

	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == note_id) {
			if (from_left) {
				int32_t new_ms = (int32_t)state->notes[i].ms - delta_duration_ms;
				int32_t new_duration =
					(int32_t)state->notes[i].duration_ms + delta_duration_ms;

				if (new_ms < 0)
					new_ms = 0;
				if (new_duration < 10)
					new_duration = 10;

				state->notes[i].ms = (uint32_t)new_ms;
				state->notes[i].duration_ms = (uint16_t)new_duration;
			} else {
				int32_t new_duration =
					(int32_t)state->notes[i].duration_ms + delta_duration_ms;
				if (new_duration < 10)
					new_duration = 10;
				state->notes[i].duration_ms = (uint16_t)new_duration;
			}
			break;
		}
	}

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		app_state_sync_notes_to_sequencer(state, sequencer, global_context->audio);
	}

	return 0;
}

static int lua_api_delete_note(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint32_t note_id = (uint32_t)luaL_checkinteger(L, 1);
	struct app_state *state = global_context->app_state;

	for (uint32_t i = 0; i < state->note_count; i++) {
		if (state->notes[i].id == note_id) {
			for (uint32_t j = i; j < state->note_count - 1; j++) {
				state->notes[j] = state->notes[j + 1];
			}
			state->note_count--;

			for (uint32_t j = 0; j < state->selection.count; j++) {
				if (state->selection.selected_ids[j] == note_id) {
					for (uint32_t k = j; k < state->selection.count - 1; k++) {
						state->selection.selected_ids[k] =
							state->selection.selected_ids[k + 1];
					}
					state->selection.count--;
					break;
				}
			}

			break;
		}
	}

	struct sequencer *sequencer = audio_get_sequencer(global_context->audio);
	if (sequencer != NULL) {
		app_state_sync_notes_to_sequencer(state, sequencer, global_context->audio);
	}

	return 0;
}

static int lua_api_play_preview_note(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL ||
	    global_context->audio == NULL) {
		return luaL_error(L, "API context not available");
	}

	uint8_t piano_key = (uint8_t)luaL_checkinteger(L, 1);
	struct app_state *state = global_context->app_state;
	struct synth *synth = audio_get_synth(global_context->audio);

	if (synth == NULL) {
		return luaL_error(L, "Synth not available");
	}

	if (state->selected_instrument >= state->instrument_count) {
		return luaL_error(L, "Invalid selected instrument");
	}

	struct instrument *inst = &state->instruments[state->selected_instrument];

	struct note_params params = {
		.frequency = note_to_frequency(piano_key),
		.duration_ms = (float)inst->default_duration_ms,
		.waveform = inst->waveform,
		.duty_cycle = inst->duty_cycle,
		.decay = inst->decay,
		.amplitude_dbfs = inst->amplitude_dbfs,
		.nes_noise_period = 15,
		.nes_noise_mode_flag = inst->nes_noise_mode_flag,
		.voice_index = -1,
		.piano_key = piano_key,
		.restart_phase = true,
		.nes_noise_lfsr_init = inst->nes_noise_lfsr
	};

	synth_play_note(synth, params);

	return 0;
}

static SDL_Scancode key_to_scancode(enum key key)
{
	switch (key) {
	case KEY_0:
		return SDL_SCANCODE_0;
	case KEY_1:
		return SDL_SCANCODE_1;
	case KEY_2:
		return SDL_SCANCODE_2;
	case KEY_3:
		return SDL_SCANCODE_3;
	case KEY_4:
		return SDL_SCANCODE_4;
	case KEY_5:
		return SDL_SCANCODE_5;
	case KEY_6:
		return SDL_SCANCODE_6;
	case KEY_7:
		return SDL_SCANCODE_7;
	case KEY_8:
		return SDL_SCANCODE_8;
	case KEY_9:
		return SDL_SCANCODE_9;
	case KEY_A:
		return SDL_SCANCODE_A;
	case KEY_B:
		return SDL_SCANCODE_B;
	case KEY_C:
		return SDL_SCANCODE_C;
	case KEY_D:
		return SDL_SCANCODE_D;
	case KEY_E:
		return SDL_SCANCODE_E;
	case KEY_F:
		return SDL_SCANCODE_F;
	case KEY_G:
		return SDL_SCANCODE_G;
	case KEY_H:
		return SDL_SCANCODE_H;
	case KEY_I:
		return SDL_SCANCODE_I;
	case KEY_J:
		return SDL_SCANCODE_J;
	case KEY_K:
		return SDL_SCANCODE_K;
	case KEY_L:
		return SDL_SCANCODE_L;
	case KEY_M:
		return SDL_SCANCODE_M;
	case KEY_N:
		return SDL_SCANCODE_N;
	case KEY_O:
		return SDL_SCANCODE_O;
	case KEY_P:
		return SDL_SCANCODE_P;
	case KEY_Q:
		return SDL_SCANCODE_Q;
	case KEY_R:
		return SDL_SCANCODE_R;
	case KEY_S:
		return SDL_SCANCODE_S;
	case KEY_T:
		return SDL_SCANCODE_T;
	case KEY_U:
		return SDL_SCANCODE_U;
	case KEY_V:
		return SDL_SCANCODE_V;
	case KEY_W:
		return SDL_SCANCODE_W;
	case KEY_X:
		return SDL_SCANCODE_X;
	case KEY_Y:
		return SDL_SCANCODE_Y;
	case KEY_Z:
		return SDL_SCANCODE_Z;
	case KEY_SPACE:
		return SDL_SCANCODE_SPACE;
	case KEY_ESCAPE:
		return SDL_SCANCODE_ESCAPE;
	case KEY_ENTER:
		return SDL_SCANCODE_RETURN;
	case KEY_TAB:
		return SDL_SCANCODE_TAB;
	case KEY_DELETE:
		return SDL_SCANCODE_DELETE;
	case KEY_BACKSPACE:
		return SDL_SCANCODE_BACKSPACE;
	case KEY_LEFT_SHIFT:
		return SDL_SCANCODE_LSHIFT;
	case KEY_RIGHT_SHIFT:
		return SDL_SCANCODE_RSHIFT;
	case KEY_LEFT_CONTROL:
		return SDL_SCANCODE_LCTRL;
	case KEY_RIGHT_CONTROL:
		return SDL_SCANCODE_RCTRL;
	case KEY_LEFT_ALT:
		return SDL_SCANCODE_LALT;
	case KEY_RIGHT_ALT:
		return SDL_SCANCODE_RALT;
	default:
		return SDL_SCANCODE_UNKNOWN;
	}
}

static int lua_api_is_key_down(lua_State *L)
{
	if (global_context == NULL || global_context->graphics == NULL) {
		return luaL_error(L, "API context not available");
	}

	const char *key_str = luaL_checkstring(L, 1);
	enum key key = parse_key_string(key_str);

	if (key == KEY_COUNT) {
		lua_pushboolean(L, false);
		return 1;
	}

	struct window *window = graphics_get_window(global_context->graphics);
	if (window == NULL) {
		lua_pushboolean(L, false);
		return 1;
	}

	SDL_Scancode scancode = key_to_scancode(key);
	if (scancode == SDL_SCANCODE_UNKNOWN) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool is_down = window_is_scancode_down(window, scancode);
	lua_pushboolean(L, is_down);

	return 1;
}

static int lua_api_zoom_horizontal_at_mouse(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	float factor = (float)luaL_checknumber(L, 1);
	float mouse_x = (float)luaL_checknumber(L, 2);

	app_state_zoom_horizontal_at_mouse(global_context->app_state, factor, mouse_x);

	return 0;
}

static int lua_api_zoom_vertical_at_mouse(lua_State *L)
{
	if (global_context == NULL || global_context->app_state == NULL) {
		return luaL_error(L, "API context not available");
	}

	float factor = (float)luaL_checknumber(L, 1);
	float mouse_y = (float)luaL_checknumber(L, 2);

	app_state_zoom_vertical_at_mouse(global_context->app_state, factor, mouse_y);

	return 0;
}

void lua_api_register_all(struct lua_runtime *runtime, struct lua_api_context *ctx)
{
	if (runtime == NULL || runtime->L == NULL) {
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

	lua_pushcfunction(runtime->L, lua_api_hex_to_rgb);
	lua_setfield(runtime->L, -2, "hexToRgb");

	lua_pushcfunction(runtime->L, lua_api_is_point_in_rect);
	lua_setfield(runtime->L, -2, "isPointInRect");

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

	lua_pushcfunction(runtime->L, lua_api_save_c);
	lua_setfield(runtime->L, -2, "saveC");

	lua_pushcfunction(runtime->L, lua_api_save_wav);
	lua_setfield(runtime->L, -2, "saveWav");

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

	lua_pushcfunction(runtime->L, lua_api_get_fps);
	lua_setfield(runtime->L, -2, "getFps");

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

	lua_pushcfunction(runtime->L, lua_api_set_voice_hidden);
	lua_setfield(runtime->L, -2, "setVoiceHidden");

	lua_pushcfunction(runtime->L, lua_api_set_voice_solo);
	lua_setfield(runtime->L, -2, "setVoiceSolo");

	lua_pushcfunction(runtime->L, lua_api_set_voice_muted);
	lua_setfield(runtime->L, -2, "setVoiceMuted");

	lua_pushcfunction(runtime->L, lua_api_set_selected_voice);
	lua_setfield(runtime->L, -2, "setSelectedVoice");

	lua_pushcfunction(runtime->L, lua_api_set_note_voice);
	lua_setfield(runtime->L, -2, "setNoteVoice");

	lua_pushcfunction(runtime->L, lua_api_get_app_state);
	lua_setfield(runtime->L, -2, "getAppState");

	lua_pushcfunction(runtime->L, lua_api_get_mouse_position);
	lua_setfield(runtime->L, -2, "getMousePosition");

	lua_pushcfunction(runtime->L, lua_api_is_mouse_button_down);
	lua_setfield(runtime->L, -2, "isMouseButtonDown");

	lua_pushcfunction(runtime->L, lua_api_is_mouse_button_pressed);
	lua_setfield(runtime->L, -2, "isMouseButtonPressed");

	lua_pushcfunction(runtime->L, lua_api_get_instrument_count);
	lua_setfield(runtime->L, -2, "getInstrumentCount");

	lua_pushcfunction(runtime->L, lua_api_get_instrument);
	lua_setfield(runtime->L, -2, "getInstrument");

	lua_pushcfunction(runtime->L, lua_api_set_selected_instrument);
	lua_setfield(runtime->L, -2, "setSelectedInstrument");

	lua_pushcfunction(runtime->L, lua_api_is_note_in_scale);
	lua_setfield(runtime->L, -2, "isNoteInScale");

	lua_pushcfunction(runtime->L, lua_api_is_root_note);
	lua_setfield(runtime->L, -2, "isRootNote");

	lua_pushcfunction(runtime->L, lua_api_ms_to_x);
	lua_setfield(runtime->L, -2, "msToX");

	lua_pushcfunction(runtime->L, lua_api_x_to_ms);
	lua_setfield(runtime->L, -2, "xToMs");

	lua_pushcfunction(runtime->L, lua_api_piano_key_to_y);
	lua_setfield(runtime->L, -2, "pianoKeyToY");

	lua_pushcfunction(runtime->L, lua_api_y_to_piano_key);
	lua_setfield(runtime->L, -2, "yToPianoKey");

	lua_pushcfunction(runtime->L, lua_api_get_selection);
	lua_setfield(runtime->L, -2, "getSelection");

	lua_pushcfunction(runtime->L, lua_api_clear_selection);
	lua_setfield(runtime->L, -2, "clearSelection");

	lua_pushcfunction(runtime->L, lua_api_select_note);
	lua_setfield(runtime->L, -2, "selectNote");

	lua_pushcfunction(runtime->L, lua_api_deselect_note);
	lua_setfield(runtime->L, -2, "deselectNote");

	lua_pushcfunction(runtime->L, lua_api_is_note_selected);
	lua_setfield(runtime->L, -2, "isNoteSelected");

	lua_pushcfunction(runtime->L, lua_api_move_note);
	lua_setfield(runtime->L, -2, "moveNote");

	lua_pushcfunction(runtime->L, lua_api_resize_note);
	lua_setfield(runtime->L, -2, "resizeNote");

	lua_pushcfunction(runtime->L, lua_api_delete_note);
	lua_setfield(runtime->L, -2, "deleteNote");

	lua_pushcfunction(runtime->L, lua_api_play_preview_note);
	lua_setfield(runtime->L, -2, "playPreviewNote");

	lua_pushcfunction(runtime->L, lua_api_is_key_down);
	lua_setfield(runtime->L, -2, "isKeyDown");

	lua_pushcfunction(runtime->L, lua_api_zoom_horizontal_at_mouse);
	lua_setfield(runtime->L, -2, "zoomHorizontalAtMouse");

	lua_pushcfunction(runtime->L, lua_api_zoom_vertical_at_mouse);
	lua_setfield(runtime->L, -2, "zoomVerticalAtMouse");

	lua_pushinteger(runtime->L, SDL_BUTTON_LEFT);
	lua_setfield(runtime->L, -2, "MOUSE_BUTTON_LEFT");

	lua_pushinteger(runtime->L, SDL_BUTTON_MIDDLE);
	lua_setfield(runtime->L, -2, "MOUSE_BUTTON_MIDDLE");

	lua_pushinteger(runtime->L, SDL_BUTTON_RIGHT);
	lua_setfield(runtime->L, -2, "MOUSE_BUTTON_RIGHT");

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
