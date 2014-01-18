/*
 * button.h
 *
 *  Created on: 30. juni 2013
 *      Author: Mathias
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "SDL.h"
#include "../graphics/sprite.h"
#include "../graphics/draw.h"

#include "touch.h"

typedef touchable *button;
typedef void (*btn_click_callback)(void *click_data);
typedef int (*btn_drag_callback)(button btn_id, SDL_TouchFingerEvent *finger, void *drag_data);

//TODO add timeout for move and up?
button button_create(SPRITE_ID spr_id, int stretch, const char *text, float pos_x, float pos_y, float width, float height);
void button_free(button btn);

void button_set_txt_antirot(button btn_id, int antirot);
void button_set_click_callback(button btn_id, btn_click_callback click_callback, void *click_data);
void button_set_drag_callback(button btn_id, btn_drag_callback drag_callback, void *drag_data);
void button_set_hotkeys(button btn_id, SDL_Scancode key, SDL_Scancode key_alt);

void button_set_backcolor(button btn_id, Color col);
void button_set_frontcolor(button btn_id, Color col);

void button_set_animated(button btn_id, int animated, float fps);
void button_set_enlargement(button btn_id, float size);
void button_set_sprite(button btn_id, SPRITE_ID spr_id);
void button_set_font(button btn_id, bm_font *f, float size);
void button_set_text(button btn_id, char *str);

sprite *button_get_sprite(button btn_id);

void button_clear(button btn);

int button_isdown(button btn_id);

void button_click(button btn);


#endif /* BUTTON_H_ */
