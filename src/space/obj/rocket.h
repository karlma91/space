#ifndef ROCKET_H_
#define ROCKET_H_

#include "objects.h"
#include "tankfactory.h"
#include "../../engine/components/hpbar.h"

extern object_group_preset type_rocket;

struct _object_param_rocket {
	float max_hp;
	int score;
	int tex_id;
	float force;
};

typedef struct {
	/* standard */
	object_data data;

	/* rocket */
	cpShape *shape;
	object_group_factory *factory;
	int factory_id;
	float timer;
	float angle;
	int active;
	float rot_speed;
	hpbar hp_bar;
	emitter *flame;

	/*parameters */
	object_param_rocket *param;
} object_group_rocket;

extern object_group_rocket *object_create_rocket(float xpos,object_group_factory *factory, object_param_rocket *param);

#endif /* ROCKET_H_ */
