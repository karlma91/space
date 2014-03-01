/*
 * spacelvl.h
 *
 *  Created on: 13. feb. 2014
 *      Author: mathiahw
 */

#ifndef SPACELVL_H_
#define SPACELVL_H_

#include "../solarsystem.h"
#include "../tilemap.h"

typedef struct spacelvl {
	char name[30];

	int timelimit;
	float height;
	float inner_radius;
	float outer_radius;

	LList ll_recipes;
	tilemap tm;

	/* level floor, ceiling and tiles shapes NB! Not transferable between states! */
	LList ll_tileshapes;
	cpShape *ceiling;
	cpSpace *loaded2space;
} spacelvl;

extern param_list param_defs;

spacelvl *spacelvl_parse(int dirtype, const char *path);
we_bool spacelvl_write(spacelvl *lvl);
spacelvl *spacelvl_copy(spacelvl *lvl);
we_bool spacelvl_load2state(spacelvl *lvl);
we_bool spacelvl_unload2state(spacelvl *lvl);
we_bool spacelvl_freecopy(spacelvl **lvl);
void spacelvl_free(spacelvl *slvl);

#endif /* SPACELVL_H_ */
