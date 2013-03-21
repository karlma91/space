/*
 * hpbar.h
 *
 *  Created on: Mar 20, 2013
 *      Author: mathiahw
 */

#ifndef HPBAR_H_
#define HPBAR_H_

#include "chipmunk.h"

typedef struct {
	float value;
	float max_hp;
	float hp_last;
	float hp_timer;
	float width;
	float height;
	float x_offset;
	float y_offset;
	cpFloat *x;
	cpFloat *y;
} hpbar;

void hpbar_init(hpbar *hp_bar, float max_hp, float width, float height, float x_offset, float y_offset, cpVect *pos);
extern void hpbar_draw(hpbar *hp_bar);

#endif /* HPBAR_H_ */
