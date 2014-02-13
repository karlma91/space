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
	station *metadata;
	int timelimit;

	float height;
	float inner_radius;
	float outer_radius;

	LList ll_recipes;
	tilemap tm;
} spacelvl;

spacelvl *spacelvl_parse(int dirtype, const char *path);
we_bool spacelvl_write(spacelvl *lvl);
we_bool spacelvl_copy(spacelvl *lvl);

#endif /* SPACELVL_H_ */
