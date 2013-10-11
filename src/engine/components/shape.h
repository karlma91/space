#ifndef POLYSHAPE_H_
#define POLYSHAPE_H_

#include "../data/llist.h"
#include "chipmunk.h"

typedef void * POLYSHAPE_ID; /* Peker til konstant sprite data*/

typedef struct {
	LList outlines;
	LList shape;
} polygon_ins;

typedef struct {
	int num;
	cpVect *vertices;
} vertex_array;

typedef LList polyshape;

void shape_destroy(polyshape p);
polyshape shape_read(char *filename);
polyshape shape_link(char *name);
void shape_add_shapes(cpSpace *space, polyshape p, cpBody * body, int size, cpVect offset, float friction, float elasticity, cpGroup group, cpCollisionType type, cpLayers layer, unsigned int shapes);

#endif /* POLYSHAPE_H_ */
