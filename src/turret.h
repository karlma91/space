/*
 * turret.h
 *
 *  Created on: Apr 16, 2013
 *      Author: karlmka
 */

#ifndef TURRET_H_
#define TURRET_H_

#include "objects.h"
#include "hpbar.h"

extern object_group_preset type_turret;

struct _object_param_turret {
	float max_hp;
	int score;
	float rot_speed;
	float shoot_interval;
	int burst_number;
	int tex_id;
};

typedef struct {
	/* standard */
	object_data data;

	/* turret */
	cpShape *shape;
	float timer;
	float barrel_angle;
	int bullets;
	int shooting;
	float rate;
	hpbar hp_bar;

	/*parameters */
	object_param_turret *param;
} object_group_turret;

extern object_group_turret *object_create_turret(float xpos, object_param_turret *param);

#endif /* TURRET_H_ */
