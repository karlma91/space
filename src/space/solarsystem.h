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
#include "we.h"

typedef struct level_ship {
	int id;
	cpVect pos;
	float radius;
	float rotation;
	float rotation_speed;
	char  level_path[256];
	button *btn;
} station;

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
	cpVect origo;
	drawbl_sun sun;
	int station_count;
	LList stations;
} solarsystem;


solarsystem *solarsystem_create(int solsys_index, float star_size, SPRITE_ID star_spr, Color star_base, Color star_glow, Color star_add1, Color star_add2);
void solarsystem_add_station(solarsystem * sol, SPRITE_ID spr_id, int dir_type, char * name, char * path);
void solarsystem_update(solarsystem *sun);
void solarsystem_draw(solarsystem *sun);
void sun_render(int layer, drawbl_sun *sun);
void solarsystem_destroy(solarsystem *sun);

#endif /* SOLARSYSTEM_H_ */
