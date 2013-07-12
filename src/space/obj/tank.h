#ifndef TANK_H_
#define TANK_H_

#include "objects.h"
#include "tankfactory.h"
#include "../../engine/components/hpbar.h"

extern object_group_preset type_tank;

struct _object_param_tank {
	float max_hp;
	int score;
};

typedef struct {
	/* standard */
	object_data data;

	/* tank */
	cpShape *shape;
	float timer;
	float barrel_angle;
	float rot_speed;
	float max_distance;
	hpbar hp_bar;
	object_group_factory *factory;
	int factory_id;
	cpBody *wheel1;
	cpBody *wheel2;
	int state;
	sprite wheel_sprite;
	sprite turret_sprite;
	/* tmp debug variables */
	cpFloat debug_left_dist;
	cpFloat debug_right_dist;
	/* end of tmp debug variables*/

	/*parameters */
	object_param_tank *param;
} object_group_tank;

extern object_group_tank *object_create_tank(float xpos, object_group_factory *factory, object_param_tank *param);

#endif /* TANK_H_ */
