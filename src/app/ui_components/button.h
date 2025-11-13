#ifndef BUTTON_H
#define BUTTON_H

struct Graphics;

struct button
{
	const char *text;
	int x, y;
	int width, height;
};

void button_render(struct Graphics *graphics, struct button *button);

#endif