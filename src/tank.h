#ifndef TANK_H_
#define TANK_H_

#include "objects.h"
#include "tankfactory.h"

object *tank_init( struct tank_factory *fac, float max_hp);

extern struct obj_type type_tank;

struct tank {
	/*standard*/
	struct obj_type *type;
	int id;
	int *remove;
	int alive;
	cpBody *body;

	/*tank*/
	cpShape *shape;
	float max_hp;
	float hp;
	struct tank_factory *factory;
};

#endif /* TANK_H_ */
