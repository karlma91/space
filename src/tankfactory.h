#ifndef TANKFACTORY_H_
#define TANKFACTORY_H_

#include "chipmunk.h"
#include "objects.h"
#include "particles.h"
#include "hpbar.h"

/* the super tank_factory object */
extern object_group_preset type_tank_factory;

struct _object_param_tankfactory {
	int max_tanks;
	float max_hp;
	float spawn_delay;
	int score;
	object_param_tank *t_param;
	int tex_id;
};

typedef struct {
	/* object */
	object_data obj;

	/*tank factory*/
	cpShape *shape;
	cpFloat timer;
	int cur;
	hpbar hp_bar;

	/* animation */
	float rot;
	emitter *smoke;

	/* paramerters */
	object_param_tankfactory *param;
} object_group_tankfactory;

object_data* tankfactory_init(int x_pos, object_param_tankfactory *param);

#endif /* TANKFACTORY_H_ */
