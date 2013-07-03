/*
 * button.c
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#include "button.h"
#include "main.h"
#include "draw.h"

//TODO support label/text on button and extendible texture button (like how bullets are drawn)

#define inside(btn,px,py) ((px >= btn->p1x) && (px <= btn->p2x) && (py >= btn->p1y) && (py <= btn->p2y))

struct button {
	SDL_FingerID finger_id;

	int pressed; /* whether if button is currently pressed down */

	float pos_x; /* center x-coordinate of button */
	float pos_y; /* center y-coordinate of button */

	int tex_id; /* texture id */
	float width; /* width of button and touch area */
	float height; /* height og button and touch area */

	float p1x;
	float p1y;
	float p2x;
	float p2y;

	button_type type;
};

button button_create(float pos_x, float pos_y, float width, float height, int tex_id, button_type type)
{
	struct button *btn = malloc(sizeof(*btn));

	btn->pos_x = pos_x;
	btn->pos_y = pos_y;
	btn->width = width;
	btn->height = height;
	btn->tex_id = tex_id;

	float margin = (btn->width < btn->height ? btn->width : btn->height) / 10;
	btn->p1x = btn->pos_x - (btn->width/2 + margin);
	btn->p2x = btn->pos_x + (btn->width/2 + margin);
	btn->p1y = btn->pos_y - (btn->height/2 + margin);
	btn->p2y = btn->pos_y + (btn->height/2 + margin);

	button_clear(btn);

	return btn;
}

void button_free(button btn_id)
{
	free(btn_id);
}

void button_render(button btn_id)
{
	struct button *btn = (struct button *) btn_id;

	float size = 1.0f; //tmp visual change, TODO support two-state button graphic (up and down)
	size = btn->type == (BTN_IMAGE_SIZED || BTN_IMAGE_SIZED_TEXT) && btn->pressed ? 1.5f : size;

	cpVect btn_pos = {btn->pos_x,btn->pos_y};
	draw_texture(btn->tex_id, &btn_pos, TEX_MAP_FULL, btn->width*size, btn->height*size, 0);
}

void button_set_texture(button btn_id, int tex_id)
{
	struct button *btn = (struct button *) btn_id;
	btn->tex_id = tex_id;
}

int button_isdown(button btn_id)
{
	struct button *btn = (struct button *) btn_id;
	return btn->pressed;
}

void button_clear(button btn_id)
{
	struct button *btn = (struct button *) btn_id;

	btn->finger_id = 0;
	btn->pressed = 0;
}

int button_finger_down(button btn_id, SDL_TouchFingerEvent *finger)
{
	//TODO check if registered touch_id corresponds to current touch_id, to clear uncaught release

	struct button *btn = (struct button *) btn_id;

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (inside(btn, tx, ty)) {
		btn->finger_id = finger->fingerId;
		btn->pressed = 1;
		return 1;
	}

	return 0;
}

int button_finger_move(button btn_id, SDL_TouchFingerEvent *finger)
{
	struct button *btn = (struct button *) btn_id;

	if (!btn->pressed || btn->finger_id != finger->fingerId)
		return 0;

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (!inside(btn, tx,ty)) {
		button_clear(btn_id);
		return 0;
	}

	return 1;
}

//TODO s¿rg for at pressed settes til 0 ved state change?
int button_finger_up(button btn_id, SDL_TouchFingerEvent *finger)
{
	struct button *btn = (struct button *) btn_id;

	if (!btn->pressed || btn->finger_id != finger->fingerId)
		return 0;

	button_clear(btn_id);

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (inside(btn, tx,ty)) {
		return 1;
	}

	return 0;
}

