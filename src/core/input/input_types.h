#ifndef INPUT_TYPES_H
#define INPUT_TYPES_H

#include <stdbool.h>
#include <stdint.h>

enum key {
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_SPACE,
	KEY_TAB,
	KEY_ESCAPE,
	KEY_DELETE,
	KEY_ENTER,
	KEY_BACKSPACE,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_PLUS,
	KEY_MINUS,
	KEY_EQUALS,
	KEY_LEFT_BRACKET,
	KEY_RIGHT_BRACKET,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_COLON,
	KEY_SEMICOLON,
	KEY_SLASH,
	KEY_BACKSLASH,
	KEY_APOSTROPHE,
	KEY_GRAVE,
	KEY_LEFT_SHIFT,
	KEY_RIGHT_SHIFT,
	KEY_LEFT_CONTROL,
	KEY_RIGHT_CONTROL,
	KEY_LEFT_ALT,
	KEY_RIGHT_ALT,
	KEY_COUNT
};

enum mouse_button {
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_COUNT
};

enum input_event_type {
	INPUT_EVENT_KEY_DOWN,
	INPUT_EVENT_KEY_UP,
	INPUT_EVENT_MOUSE_DOWN,
	INPUT_EVENT_MOUSE_UP,
	INPUT_EVENT_MOUSE_MOVE,
	INPUT_EVENT_SCROLL
};

struct input_event_key_down {
	enum key key;
	bool shift;
	bool ctrl;
	bool alt;
};

struct input_event_key_up {
	enum key key;
};

struct input_event_mouse_down {
	float x, y;
	enum mouse_button button;
	bool shift;
	bool ctrl;
	bool alt;
};

struct input_event_mouse_up {
	float x, y;
	enum mouse_button button;
};

struct input_event_mouse_move {
	float x, y;
};

struct input_event_scroll {
	float dx, dy;
	bool shift;
	bool ctrl;
	bool alt;
};

struct input_event {
	enum input_event_type type;
	union {
		struct input_event_key_down key_down;
		struct input_event_key_up key_up;
		struct input_event_mouse_down mouse_down;
		struct input_event_mouse_up mouse_up;
		struct input_event_mouse_move mouse_move;
		struct input_event_scroll scroll;
	} data;
};

#endif
