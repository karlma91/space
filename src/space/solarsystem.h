/*
 * solarsystem.h
 *
 *  Created on: 10. des. 2013
 *      Author: mathiahw
 */

#ifndef SOLARSYSTEM_H_
#define SOLARSYSTEM_H_

#include "we_utils.h"
#include "we_graphics.h"

typedef struct {
	float size;
	SPRITE_ID spr_id;
	Color base, glow, add1, add2;
	float angvel, ang1, ang2;
	cpVect pos;
} drawbl_sun;

/* representing a space station system orbitting a sun */
typedef struct solarsystem {
	int index;
	view *cam;
	cpVect origo;
	drawbl_sun sun;
	int station_count;
} solarsystem;


solarsystem *solarsystem_create(view *cam, int solsys_index, float star_size, SPRITE_ID star_spr, Color star_base, Color star_glow, Color star_add1, Color star_add2);
void solarsystem_add_station(SPRITE_ID spr_id);
void solarsystem_update(solarsystem *sun);
void solarsystem_draw(solarsystem *sun);
void sun_render(int layer, drawbl_sun *sun);

#endif /* SOLARSYSTEM_H_ */
