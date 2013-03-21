#ifndef TANK_H_
#define TANK_H_

#include "objects.h"
#include "tankfactory.h"
#include "hpbar.h"


extern struct obj_type type_tank;

struct tank;
struct tank_param;

extern object *tank_init(float xpos, struct tank_factory *factory, struct tank_param *param);

struct tank {
	/* standard */
	object obj;

	/* tank */
	cpShape *shape;
	float timer;
	float angle;
	float rot_speed;
	hpbar hp_bar;
	struct tank_factory *factory;
	cpBody *wheel1;
	cpBody *wheel2;

	/*parameters */
	struct tank_param *param;
};

struct tank_param {
	float max_hp;
	int score;
	int tex_id;
};

#endif /* TANK_H_ */
