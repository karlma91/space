#ifndef POLYSHAPE_H_
#define POLYSHAPE_H_

#include "../engine/data/llist.h"
#include "chipmunk.h"

typedef void * POLYSHAPE_ID; /* Peker til konstant sprite data*/

typedef struct {
	int shapes;
	LList polylist;
} polyshape;

typedef struct {
	int num;
	cpVect *shape;
} shape_instance;

void polyshape_destroy(polyshape *p);
polyshape * polyshape_read(char *filename);
void polyshape_add_shapes(polyshape *p, cpBody * body, int size, float friction, float elasticity, int group, int type, int layer);

#endif /* POLYSHAPE_H_ */
