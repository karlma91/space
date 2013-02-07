#ifndef TANKFACTORY_H_
#define TANKFACTORY_H_

#include "chipmunk.h"
#include "objects.h"


/* the super tank_factory object */
extern struct obj_type type_tank_factory;

struct tank_factory_param;

object* tankfactory_init(int x_pos, struct tank_factory_param *param);

struct tank_factory {
	/* object */
	object obj;

	/*tank factory*/
	cpShape *shape;
	cpFloat timer;
	int cur;
	cpFloat hp;
	struct tank_param *t_param;

	/* paramerters */
	struct tank_factory_param *param;
};

struct tank_factory_param {
	int max_tanks;
	cpFloat max_hp;
	cpFloat spawn_delay;
};


#endif /* TANKFACTORY_H_ */
