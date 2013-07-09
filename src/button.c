/*
 * button.c
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#include "button.h"
#include "main.h"
#include "draw.h"
#include "font.h"


static int BUTTON_UP = 0;
static int BUTTON_DOWN = 1;

//TODO support label/text on button and extendible texture button (like how bullets are drawn)

#define inside(btn,px,py) ((px >= btn->p1x) && (px <= btn->p2x) && (py >= btn->p1y) && (py <= btn->p2y))

struct button {
	SDL_FingerID finger_id;

	sprite spr;
	char *label;
	int stretch;

	int pressed; /* whether if button is currently pressed down */

	float pos_x; /* center x-coordinate of button */
	float pos_y; /* center y-coordinate of button */

	float width; /* width of button and touch area */
	float height; /* height og button and touch area */

	void (*callback)(void);

	/* pre-calculated touch region */
	float p1x;
	float p1y;
	float p2x;
	float p2y;
};

button button_create(SPRITE_ID spr_id, int stretch, char *text, float pos_x, float pos_y, float width, float height)
{
	struct button *btn = malloc(sizeof(*btn));

	btn->pos_x = pos_x;
	btn->pos_y = pos_y;
	btn->width = width;
	btn->height = height;

	btn->stretch = stretch;
	btn->label = text;

	btn->callback = NULL;

	sprite_create(&(btn->spr), spr_id, width, height, 0);

	float touch_margin = (btn->width < btn->height ? btn->width : btn->height) / 10;
	btn->p1x = btn->pos_x - (btn->width/2 + touch_margin);
	btn->p2x = btn->pos_x + (btn->width/2 + touch_margin);
	btn->p1y = btn->pos_y - (btn->height/2 + touch_margin);
	btn->p2y = btn->pos_y + (btn->height/2 + touch_margin);

	button_clear(btn);

	return btn;
}

void button_set_callback(button btn_id, void (*callback)(void))
{
	struct button *btn = (struct button *) btn_id;
	btn->callback = callback;
}

void button_free(button btn_id)
{
	free(btn_id);
}

void button_render(button btn_id)
{
	struct button *btn = (struct button *) btn_id;

	cpVect btn_pos = {btn->pos_x,btn->pos_y};


	if (btn->spr.id) {
		if (btn->stretch) {
			draw_glow_line(btn->p1x,btn->p1y,btn->p2x,btn->p2y,btn->height);
		} else {
			sprite_render(&(btn->spr), &btn_pos, 0);
		}
	}

	if (btn->label) {
		//TODO create font struct
		setTextAlign(TEXT_CENTER);
		setTextAngle(0);

		setTextSize(btn->height / 3);
		font_drawText(btn->pos_x, btn->pos_y, btn->label);
	}
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
	sprite_set_index(&(btn->spr), BUTTON_UP);
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
		sprite_set_index(&(btn->spr), BUTTON_DOWN);
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
		sprite_set_index(&(btn->spr), BUTTON_UP);
		button_click(btn_id);
		return 1;
	}

	return 0;
}

void button_click(button btn_id)
{
	struct button *btn = (struct button *) btn_id;
	if (btn->callback) {
		btn->callback();
	}
}
