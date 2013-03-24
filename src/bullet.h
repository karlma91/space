#ifndef BULLET_H_
#define BULLET_H_

#include "chipmunk.h"
#include "draw.h"
#include "objects.h"
#include "particles.h"
#include "space.h"

object_data *bullet_init(cpVect pos, cpVect dir, cpVect intit_vel, int type);

extern object_group_preset type_bullet_player;
extern object_group_preset type_bullet_enemy;

struct bullet{
	/*object*/
	object_group_preset *type;
	int instance_id;
	int *remove;
	int alive;
	cpBody *body;
	/*Bullet*/
	cpShape *shape;
	int bullet_type;
	cpFloat damage;
	cpFloat speed;
	Color color;
};

#endif /* BULLET_H_ */
