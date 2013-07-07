/*
 * button.c
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#include "button.h"
#include "main.h"
#include "draw.h"


static int BUTTON_UP = 0;
static int BUTTON_DOWN = 1;

//TODO support label/text on button and extendible texture button (like how bullets are drawn)

#define inside(btn,px,py) ((px >= btn->p1x) && (px <= btn->p2x) && (py >= btn->p1y) && (py <= btn->p2y))

struct button {
	SDL_FingerID finger_id;

	int pressed; /* whether if button is currently pressed down */

	float pos_x; /* center x-coordinate of button */
	float pos_y; /* center y-coordinate of button */

	sprite spr;
	float width; /* width of button and touch area */
	float height; /* height og button and touch area */

	float p1x;
	float p1y;
	float p2x;
	float p2y;

	button_type type;
};

button button_create(float pos_x, float pos_y, float width, float height, SPRITE_ID sprid, button_type type)
{
	struct button *btn = malloc(sizeof(*btn));

	btn->pos_x = pos_x;
	btn->pos_y = pos_y;
	btn->width = width;
	btn->height = height;
	sprite_create(&(btn->spr), sprid, width, height, 0);
	btn->type = type;

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

	cpVect btn_pos = {btn->pos_x,btn->pos_y};
	//draw_texture(btn->tex_id, &btn_pos, TEX_MAP_FULL, btn->width*size, btn->height*size, 0);

	if (btn->type == BTN_SPRITE) {
		if (btn->pressed) {
			button_set_texture(btn, BUTTON_DOWN);
		} else {
			button_set_texture(btn, BUTTON_UP);
		}
	}
	if (((btn->type == BTN_IMAGE_SIZED) || (btn->type == BTN_IMAGE_SIZED_TEXT)) && btn->pressed) {
		float width = btn->spr.width;
		float height = btn->spr.height;
		btn->spr.width *=1.5;
		btn->spr.height *=1.5;
		sprite_render(&(btn->spr), &btn_pos, 0);
		btn->spr.width = width;
		btn->spr.height = height;
	} else {
		sprite_render(&(btn->spr), &btn_pos, 0);
	}
}

void button_set_texture(button btn_id, int tex_id)
{
	struct button *btn = (struct button *) btn_id;
	sprite_set_index(&(btn->spr), tex_id);
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

//TODO s�rg for at pressed settes til 0 ved state change?
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

