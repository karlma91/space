/*
 * button.c
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */
#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"

#include "button.h"

#include <string.h>
#include "../engine.h"
#include "../data/llist.h"
#include "../graphics/draw.h"
#include "../graphics/bmfont.h"
#include "../../space/game.h"
#include "we_utils.h"



static int BUTTON_UP = 0;
static int BUTTON_DOWN = 1;

//TODO be able to manually set text size for text buttons

struct button {
	touchable touch_data;

	touch_unique_id touch_id;

	sprite spr;
	char label[50];
	int stretch;

	int pressed; /* whether if button is currently pressed down or not */

	void (*callback)(void *);

	int animated;
	float down_size, current_size;

	float font_size;
	bm_font *font;

	Color backcol;
	Color frontcol;

	SDL_Scancode key1, key2;

	void *data;
};

static int keypress_down(button btn_id, SDL_Scancode key)
{
	struct button *btn = (struct button *) btn_id;

	if (key == btn->key1 || key == btn->key2) {
		button_click(btn_id);
		return 1;
	}

	return 0;
}

button button_create(SPRITE_ID spr_id, int stretch, const char *text, float pos_x, float pos_y, float width, float height)
{
	struct button *btn = calloc(1, sizeof *btn);

	REGISTER_BUTTON_CALLS(btn);

	btn->touch_data.type = CTRL_BUTTON;

	btn->touch_data.get.width = width;
	btn->touch_data.get.height = height;

	btn->backcol = COL_WHITE;
	btn->frontcol = COL_WHITE;

	btn->stretch = stretch;
	if (text) {
		strcpy(btn->label, text);
	} else {
		btn->label[0] = 0;
	}

	btn->animated = 0;
	btn->down_size = 1;
	btn->font_size = 1;
	btn->font = FONT_SANS;
	btn->current_size = btn->down_size;

	sprite_create(&(btn->spr), spr_id, width, height, 0);

	touch_place((touchable *) btn, pos_x, pos_y);
	button_clear((button) btn);

	return (button) btn;
}

void button_set_callback(button btn_id, btn_callback callback, void *data)
{
	struct button *btn = (struct button *) btn_id;
	btn->callback = callback;
	btn->data = data;
}

void button_set_hotkeys(button btn_id, SDL_Scancode key, SDL_Scancode key_alt)
{
	struct button *btn = (struct button *) btn_id;
	btn->key1 = key;
	btn->key2 = key_alt;
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
	btn->spr.sub_index += 50.0f * we_randf; //TODO hardkodet!
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

void button_set_font(button btn_id, bm_font *f, float size)
{
	struct button *btn = (struct button *) btn_id;
	btn->font = f;
	btn->font_size = size;
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
		sprite_render_scaled(1, &(btn->spr), btn_pos, 0, scale);
	} else if (btn->spr.id) {
		draw_color(btn->backcol);
		if (btn->stretch) {
			cpVect a = cpvadd(btn_pos, cpv(-width/2,0));
			cpVect b = cpvadd(btn_pos, cpv(width/2,0));
			draw_line_spr(1, &(btn->spr), a, b, height, 0);
		} else {
			sprite_render_scaled(1, &(btn->spr), btn_pos, 0, scale);
		}
	}

	if (btn->label[0]) {
		//TODO create font struct with color + size + alignment + angle
		setTextAlign(TEXT_CENTER);
		setTextAngle(0);

		draw_color(btn->frontcol);

		setTextSize(height / 4 * scale);
		//font_drawText(0, x, y + 12 * scale, btn->label);
		bmfont_center(btn->font, cpv(x, y), btn->font_size * scale, "%s", btn->label);
	}
}

static int touch_down(button btn_id, SDL_TouchFingerEvent *finger)
{
	//TODO check if registered touch_id corresponds to current touch_id, to clear uncaught release
	struct button *btn = (struct button *) btn_id;

	float tx = finger->x, ty = finger->y;
	//normalized2game(&tx, &ty);

	if (INSIDE(btn, tx, ty)) {
		finger_unbind(btn->touch_id);
		btn->touch_id = finger_bind_force(finger->fingerId);
		if (btn->touch_id != -1) {
			btn->pressed = 1;
			if (!btn->animated) sprite_set_index(&(btn->spr), BUTTON_DOWN);
			return 1;
		}
	}

	return 0;
}

static int touch_motion(button btn_id, SDL_TouchFingerEvent *finger)
{
	float tx = finger->x, ty = finger->y;
	struct button *btn = (struct button *) btn_id;
	int active = finger_status(btn->touch_id, finger->fingerId);

	if (!btn->pressed || !active)
		return 0;

	if (!INSIDE(btn, tx,ty)) {
		finger_unbind(btn->touch_id);
		button_clear(btn_id);
		return 0;
	}

	return 1;
}

static int touch_up(button btn_id, SDL_TouchFingerEvent *finger)
{
	float tx = finger->x, ty = finger->y;
	struct button *btn = (struct button *) btn_id;
	int active = finger_status(btn->touch_id, finger->fingerId);

	if (!btn->pressed || !active)
		return 0;

	button_clear(btn_id);

	if (INSIDE(btn, tx,ty)) {
		if (!btn->animated) sprite_set_index(&(btn->spr), BUTTON_UP);
		button_click(btn_id);
		return 1;
	}

	return 0;
}

