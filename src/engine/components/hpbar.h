/*
 * hpbar.h
 *
 *  Created on: Mar 20, 2013
 *      Author: mathiahw
 */

#ifndef HPBAR_H_
#define HPBAR_H_

#include "we_utils.h"
#include "chipmunk.h"

typedef struct {
	float value;
	float max_hp;
	float draw_value;
	float hp_timer;
	float width;
	float height;
	float x_offset;
	float y_offset;
	cpVect *pos;
	we_bool invinc;
} hpbar;

void hpbar_init(hpbar *hp_bar, float max_hp, float width, float height, float x_offset, float y_offset, cpVect *pos);
void hpbar_draw(int layer, hpbar *hp_bar, cpFloat angle);
void hpbar_set_invincible(hpbar *hp_bar, we_bool b);

#endif /* HPBAR_H_ */
