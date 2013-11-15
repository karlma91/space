/*
 * scroll.h
 *
 *  Created on: 16. juli 2013
 *      Author: Mathias
 */

#ifndef SCROLL_H_
#define SCROLL_H_

#include "touch.h"
#include "chipmunk.h"

typedef touchable *scroll_p;

typedef int (*pre_touch_callback) (cpVect pos);

scroll_p scroll_create(float pos_x, float pos_y, float width, float height, float friction, float max_speed, int rotate_delta, int zoom_delta, int consume_events);

void scroll_set_hotkeys(touchable * scr_id, SDL_Scancode key_left, SDL_Scancode key_up, SDL_Scancode key_right, SDL_Scancode key_down, SDL_Scancode key_zoomin, SDL_Scancode key_zoomout, SDL_Scancode key_rotcw, SDL_Scancode key_rotcc);

float scroll_get_xoffset(scroll_p);
float scroll_get_yoffset(scroll_p);
cpVect scroll_get_offset(scroll_p scr_id);
void scroll_set_offset(scroll_p scr_id, cpVect offset);
void scroll_set_bounds(scroll_p scr_id, cpBB bounds);
void scroll_set_callback(scroll_p scr_id, pre_touch_callback touch_down, pre_touch_callback touch_motion, pre_touch_callback touch_up);

float scroll_get_zoom(scroll_p);
void scroll_set_zoom(scroll_p, float zoom);
void scroll_set_zoomlimit(scroll_p scr_id, float min, float max);
float scroll_get_rotation(scroll_p);

#endif /* SCROLL_H_ */
