/*
 * button.c
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#include "button.h"
#include "main.h"
#include "draw.h"

#define inside(button,px,py) ((px >= btn->r1x) && (px <= btn->r2x) && (py >= btn->r1y) && (py <= btn->r2y))

void button_init(button *btn, float pos_x, float pos_y, float width, float height, int tex_id)
{
	btn->pos_x = pos_x;
	btn->pos_y = pos_y;
	btn->width = width;
	btn->height = height;
	btn->tex_id = tex_id;

	float margin = (btn->width < btn->height ? btn->width : btn->height) / 2;
	btn->r1x = btn->pos_x - (btn->width/2 + margin);
	btn->r2x = btn->pos_x + (btn->width/2 + margin);
	btn->r1y = btn->pos_y - (btn->height/2 + margin);
	btn->r2y = btn->pos_y + (btn->height/2 + margin);

	button_clear(btn);
}

void button_render(button *btn)
{
	float size = btn->pressed ? 1.5f : 1.0f; //tmp visual change, TODO support two-state button graphic (up and down)
	cpVect btn_pos = {btn->pos_x,btn->pos_y};
	draw_texture(TEX_BUTTON_PAUSE, &btn_pos, TEX_MAP_FULL, btn->width*size, btn->height*size, 0);
}

void button_clear(button *btn)
{
	btn->finger_id = 0;
	btn->pressed = 0;
}

int button_finger_down(button *btn, SDL_TouchFingerEvent *finger)
{
	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (inside(btn, tx, ty)) {
		btn->finger_id = finger->fingerId;
		btn->pressed = 1;
		return 1;
	}

	return 0;
}

int button_finger_move(button *btn, SDL_TouchFingerEvent *finger)
{
	if (!btn->pressed || btn->finger_id != finger->fingerId)
		return 0;

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (!inside(btn, tx,ty)) {
		button_clear(btn);
		return 0;
	}

	return 1;
}

//TODO s¿rg for at pressed settes til 0 ved state change?
int button_finger_up(button *btn, SDL_TouchFingerEvent *finger)
{
	if (!btn->pressed || btn->finger_id != finger->fingerId)
		return 0;

	button_clear(btn);

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (inside(btn, tx,ty)) {
		return 1;
	}

	return 0;
}

