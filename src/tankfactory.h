#ifndef TANKFACTORY_H_
#define TANKFACTORY_H_


#include "chipmunk.h"
#include "objects.h"

object* tankfactory_init(int x_pos, int max_tanks, float max_hp);

/* the super tank_factory object */
extern struct obj_type type_tank_factory;

struct tank_factory {
	/* object */
	struct obj_type *type;
	int instance_id;
	int *remove;
	int alive;
	cpBody *body;

	/*tank factory*/
	cpShape *shape;
	int max;
	cpFloat timer;
	int cur;
	cpFloat max_hp;
	cpFloat hp;
	float x_pos;
};


#endif /* TANKFACTORY_H_ */
