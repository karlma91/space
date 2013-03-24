#ifndef TANK_H_
#define TANK_H_

#include "objects.h"
#include "tankfactory.h"
#include "hpbar.h"

extern object_group_preset type_tank;

struct _object_param_tank {
	float max_hp;
	int score;
	int tex_id;
};

typedef struct {
	/* standard */
	object_data obj;

	/* tank */
	cpShape *shape;
	float timer;
	float angle;
	float rot_speed;
	hpbar hp_bar;
	object_group_tankfactory *factory;
	cpBody *wheel1;
	cpBody *wheel2;

	/*parameters */
	object_param_tank *param;
} object_group_tank;

extern object_data *tank_init(float xpos, object_group_tankfactory *factory, object_param_tank *param);

#endif /* TANK_H_ */
