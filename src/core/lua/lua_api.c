#include "core/lua/lua_api.h"
#include "core/graphics/color.h"
#include "core/graphics/graphics.h"
#include <stdio.h>

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
	graphics_fill_rect_outlined(global_context->graphics, x, y, width, height, outline_color,
								 outline_width);

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
	graphics_fill_rounded_rect_outlined(global_context->graphics, x, y, width, height, radius,
										 outline_color, outline_width);

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

	lua_setglobal(runtime->L, "boostio");
}

void lua_api_set_context(struct lua_api_context *ctx)
{
	global_context = ctx;
}
