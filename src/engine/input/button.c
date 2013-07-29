/*
 * button.c
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */
#include "button.h"

#include <string.h>

#include "../engine.h"
#include "../graphics/font.h"
#include "../data/llist.h"

#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"


static int BUTTON_UP = 0;
static int BUTTON_DOWN = 1;

//TODO be able to manually set text size for text buttons

struct button {
	touchable touch_data;

	SDL_FingerID finger_id;

	sprite spr;
	char label[50];
	int stretch;

	int pressed; /* whether if button is currently pressed down or not */

	void (*callback)(void *);

	int animated;
	float down_size, current_size;

	Color backcol;
	Color frontcol;

	void *data;
};

button button_create(SPRITE_ID spr_id, int stretch, const char *text, float pos_x, float pos_y, float width, float height)
{
	struct button *btn = malloc(sizeof(*btn));

	REGISTER_CALLS(btn);

	btn->touch_data.type = CTRL_BUTTON;

	btn->touch_data.get.width = width;
	btn->touch_data.get.height = height;

	btn->backcol.a=1;
	btn->backcol.r=1;
	btn->backcol.g=1;
	btn->backcol.b=1;

	btn->frontcol.a=1;
	btn->frontcol.r=1;
	btn->frontcol.g=1;
	btn->frontcol.b=1;

	btn->stretch = stretch;
	if (text) {
		strcpy(btn->label, text);
	} else {
		btn->label[0] = 0;
	}

	btn->callback = NULL;
	btn->data = NULL;

	btn->animated = 0;
	btn->down_size = 1;
	btn->current_size = btn->down_size;

	sprite_create(&(btn->spr), spr_id, width, height, 0);

	touch_place((touchable *) btn, pos_x, pos_y);

	button_clear((button) btn);

	return (button) btn;
}

void button_set_callback(button btn_id, void (*callback)(void *), void *data)
{
	struct button *btn = (struct button *) btn_id;
	btn->callback = callback;
	btn->data = data;
}


void button_set_backcolor(button btn_id, Color col)
{
	struct button *btn = (struct button *) btn_id;
	btn->backcol.a = col.a;
	btn->backcol.r = col.r;
	btn->backcol.g = col.g;
	btn->backcol.b = col.b;
}

void button_set_frontcolor(button btn_id, Color col)
{
	struct button *btn = (struct button *) btn_id;
	btn->frontcol.a = col.a;
	btn->frontcol.r = col.r;
	btn->frontcol.g = col.g;
	btn->frontcol.b = col.b;
}

void button_set_animated(button btn_id, int animated, float fps) {
	struct button *btn = (struct button *) btn_id;
	btn->spr.sub_index += 50.0f * rand() / RAND_MAX; //TODO hardkodet!
	btn->animated = animated;
	btn->spr.animation_speed = fps;
}

void button_set_enlargement(button btn_id, float size)
{
	struct button *btn = (struct button *) btn_id;
	btn->down_size = size;
}

void button_set_sprite(button btn_id, SPRITE_ID spr_id)
{
	struct button *btn = (struct button *) btn_id;
	btn->spr.id = spr_id;
}

void button_free(button btn_id)
{
	free(btn_id);
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
	if (!btn->animated) sprite_set_index(&(btn->spr), BUTTON_UP);
}

void button_click(button btn_id)
{
	struct button *btn = (struct button *) btn_id;
	if (btn->callback) {
		btn->callback(btn->data);
	}
}

static void update(button btn_id)
{
}

static void render(button btn_id)
{
	struct button *btn = (struct button *) btn_id;

	if (btn->animated) {
		sprite_update(&btn->spr);
	}

	float size = btn->pressed ? btn->down_size : 1;
	float current_size = btn->current_size;
	if (current_size != size) {
		btn->current_size = current_size * 0.7 + 0.3 * size;
	}

	float x = btn->touch_data.get.x;
	float y = btn->touch_data.get.y;

	float width = btn->touch_data.get.width;
	float height = btn->touch_data.get.height;

	cpVect btn_pos = {x,y};

	float scale = btn->current_size;

	if (btn->animated) {
		draw_color(btn->backcol);
		sprite_render_scaled(&(btn->spr), &btn_pos, 0, scale);
	} else if (btn->spr.id) {
		draw_color(btn->backcol);
		if (btn->stretch) {
			//draw_glow_line(btn->p1x,btn->p1y,btn->p2x,btn->p2y,btn->height);
			draw_sprite_line(&(btn->spr),x - width/2, y, x + width/2, y, height);
		} else {
			sprite_render_scaled(&(btn->spr), &btn_pos, 0, scale);
		}
	}

	if (btn->label) {
		//TODO create font struct with color + size + alignment + angle
		setTextAlign(TEXT_CENTER);
		setTextAngle(0);

		draw_color(btn->frontcol);

		setTextSize(height / 4 * scale);
		font_drawText(x, y + 12 * scale, btn->label);
	}
}

static int touch_down(button btn_id, SDL_TouchFingerEvent *finger)
{
	//TODO check if registered touch_id corresponds to current touch_id, to clear uncaught release
	struct button *btn = (struct button *) btn_id;

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (INSIDE(btn, tx, ty)) {
		btn->finger_id = finger->fingerId;
		btn->pressed = 1;
		if (!btn->animated) sprite_set_index(&(btn->spr), BUTTON_DOWN);
		llist_add(active_fingers, (void *)finger->fingerId);
		return 1;
	}

	return 0;
}

static int touch_motion(button btn_id, SDL_TouchFingerEvent *finger)
{
	struct button *btn = (struct button *) btn_id;

	if (!btn->pressed || btn->finger_id != finger->fingerId)
		return 0;

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (!INSIDE(btn, tx,ty)) {
		llist_remove(active_fingers, (void *) btn->finger_id); // make finger available to other touchables
		button_clear(btn_id);
		return 0;
	}

	return 1;
}

static int touch_up(button btn_id, SDL_TouchFingerEvent *finger)
{
	struct button *btn = (struct button *) btn_id;

	if (!btn->pressed || btn->finger_id != finger->fingerId)
		return 0;

	button_clear(btn_id);

	float tx = finger->x, ty = finger->y;
	normalized2game(&tx, &ty);

	if (INSIDE(btn, tx,ty)) {
		if (!btn->animated) sprite_set_index(&(btn->spr), BUTTON_UP);
		button_click(btn_id);
		return 1;
	}

	return 0;
}

