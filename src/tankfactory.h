#ifndef factory_H_
#define factory_H_

#include "chipmunk.h"
#include "objects.h"
#include "particles.h"
#include "hpbar.h"

/* the super tank_factory object */
extern object_group_preset type_tank_factory;

struct _object_param_factory {
	int max_tanks;
	float max_hp;
	float spawn_delay;
	int score;
	int type;
	object_param_tank *t_param;
	object_param_rocket *r_param;
	int tex_id;
};

typedef struct {
	/* object */
	object_data data;

	/*tank factory*/
	cpShape *shape;
	cpFloat timer;
	int cur;
	hpbar hp_bar;
	float max_distance;

	/* animation */
	float rot;
	emitter *smoke;

	/* paramerters */
	object_param_factory *param;
} object_group_factory;

object_group_factory *object_create_factory(int x_pos, object_param_factory *param);

#endif /* factory_H_ */
