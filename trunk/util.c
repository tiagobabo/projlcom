#include "util.h"
#include <stdio.h>
#include "kbc.h"

Byte byte1, byte2, byte3;
int dx, dy;//coordenadas do rato
int x_over, y_over;//overflow em x e y
int left_b, mid_b, right_b;//botões do rato
int x = 0;
int y = 0;

void parse_mouse_event()
{
	MouseData* data = (MouseData*)getGQueue(mouse_queue);
	
	byte1 = data->b1;
	byte2 = data->b2;
	byte3 = data->b3;
	
	y_over = Y_OVERFLOW(byte1);
	x_over = X_OVERFLOW(byte1);
	
	left_b = LEFT_BUTTON(byte1);
	mid_b = MIDDLE_BUTTON(byte1);
	right_b = RIGHT_BUTTON(byte1);
	
	dx = byte1 & XSGN ? (byte2 - 255) : byte2;
	dy = byte1 & YSGN ? (byte3 - 255) : byte3;
	
	x += dx;
	y += dy;
}

void process_mouse_event()
{
	printf("BE=%s BC=%s BD=%s X=%4d Y=%4d\n",
		(left_b ? "Sim" : "Nao"), (mid_b ? "Sim" : "Nao"), (right_b ? "Sim" : "Nao"), x, y);
}
