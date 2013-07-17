/*
 * scroll.h
 *
 *  Created on: 16. juli 2013
 *      Author: Mathias
 */

#ifndef SCROLL_H_
#define SCROLL_H_

#include "touch.h"

typedef touchable *scroll_p;

scroll_p scroll_create(float pos_x, float pos_y, float width, float height, float friction, float max_speed);

float scroll_get_xoffset(scroll_p);
float scroll_get_yoffset(scroll_p);

#endif /* SCROLL_H_ */
