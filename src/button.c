/*
 * button.c
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#include "string.h"
#include "button.h"
#include "main.h"
#include "draw.h"
#include "font.h"

#define THIS_IS_A_TOUCH_OBJECT 1
#include "touch.h"

static int BUTTON_UP = 0;
static int BUTTON_DOWN = 1;

//TODO support label/text on button and extendible texture button (like how bullets are drawn)

#define inside(btn,px,py) ((px >= btn->p1x) && (px <= btn->p2x) && (py >= btn->p1y) && (py <= btn->p2y))

struct button {
	const touch_calls *calls;

	SDL_FingerID finger_id;

	sprite spr;
	char label[50];
	int stretch;

	int pressed; /* whether if button is currently pressed down */

	float pos_x; /* center x-coordinate of button */
	float pos_y; /* center y-coordinate of button */

	float width; /* width of button and touch area */
	float height; /* height og button and touch area */

	void (*callback)(void *);

	/* pre-calculated touch region */
	float p1x;
	float p1y;
	float p2x;
	float p2y;

	int enabled;
	int visible;

	Color backcol;
	Color frontcol;

	void *data;
};

button button_create(SPRITE_ID spr_id, int stretch, char *text, float pos_x, float pos_y, float width, float height)
{
	struct button *btn = malloc(sizeof(*btn));

	REGISTER_CALLS(btn);

	btn->pos_x = pos_x;
	btn->pos_y = pos_y;
	btn->width = width;
	btn->height = height;

	btn->backcol.a=1;
	btn->backcol.r=1;
	btn->backcol.g=1;
	btn->backcol.b=1;

	btn->frontcol.a=1;
	btn->frontcol.r=1;
	btn->frontcol.g=1;
	btn->frontcol.b=1;

	btn->stretch = stretch;
	strcpy(btn->label, text);

	btn->callback = NULL;
	btn->data = NULL;

	btn->visible = 1;
	btn->enabled = 1;

	sprite_create(&(btn->spr), spr_id, width, height, 0);

	float touch_margin = (btn->width < btn->height ? btn->width : btn->height) / 10;
	btn->p1x = btn->pos_x - (btn->width/2 + touch_margin);
	btn->p2x = btn->pos_x + (btn->width/2 + touch_margin);
	btn->p1y = btn->pos_y - (btn->height/2 + touch_margin);
	btn->p2y = btn->pos_y + (btn->height/2 + touch_margin);

	button_clear(btn);

	return btn;
}

void button_set_callback(button btn_id, void (*callback)(void *))
{
	struct button *btn = (struct button *) btn_id;
	btn->callback = callback;
}

void button_set_data(button btn_id, void *data)
{
	struct button *btn = (struct button *) btn_id;
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

void button_set_visibility(button btn_id, int visible)
{
	struct button *btn = (struct button *) btn_id;
	btn->visible = visible;
}

void button_set_enabled(button btn_id, int enabled)
{
	struct button *btn = (struct button *) btn_id;
	btn->enabled = enabled;
}

int button_is_visible(button btn_id)
{
	return ((struct button *) btn_id)->visible;
}

int button_is_enabled(button btn_id)
{
	return ((struct button *) btn_id)->enabled;
}


void button_free(button btn_id)
{
	free(btn_id);
}

void button_render(button btn_id)
{

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

}

int button_finger_move(button btn_id, SDL_TouchFingerEvent *finger)
{

}

//TODO s�rg for at pressed settes til 0 ved state change?
int button_finger_up(button btn_id, SDL_TouchFingerEvent *finger)
{

}

void button_click(button btn_id)
{
	struct button *btn = (struct button *) btn_id;
	if (btn->callback) {
		btn->callback(btn->data);
	}
}

/* UNUSED METHOD */
static void update(button btn_id)
{
}

static void render(button btn_id)
{
	struct button *btn = (struct button *) btn_id;

	cpVect btn_pos = {btn->pos_x,btn->pos_y};


	if (btn->spr.id) {
		draw_color(btn->backcol);
		if (btn->stretch) {
			//draw_glow_line(btn->p1x,btn->p1y,btn->p2x,btn->p2y,btn->height);
			draw_sprite_line(&(btn->spr),btn->pos_x - btn->width/2, btn->pos_y, btn->pos_x + btn->width/2, btn->pos_y, btn->height);
		} else {
			sprite_render(&(btn->spr), &btn_pos, 0);
		}
	}

	if (btn->label) {
		//TODO create font struct with color + size + alignment + angle
		setTextAlign(TEXT_CENTER);
		setTextAngle(0);

		draw_color(btn->frontcol);

		setTextSize(btn->height / 4);
		font_drawText(btn->pos_x, btn->pos_y + 12, btn->label);
	}
}

static int touch_down(button btn_id, SDL_TouchFingerEvent *finger)
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

static int touch_motion(button btn_id, SDL_TouchFingerEvent *finger)
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

static int touch_up(button btn_id, SDL_TouchFingerEvent *finger)
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

