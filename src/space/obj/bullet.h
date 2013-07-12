#ifndef BULLET_H_
#define BULLET_H_

#include "chipmunk.h"

#include "../../engine/graphics/draw.h"
#include "objects.h"

object_data *object_create_bullet(cpVect pos, cpVect dir, cpVect intit_vel, int type);

extern object_group_preset type_bullet_player;
extern object_group_preset type_bullet_enemy;

struct bullet{
	/* standard */
	object_data data;
	/*Bullet*/
	cpShape *shape;
	int bullet_type;
	float damage;
	cpFloat speed;
	Color color;
	float energy;
};

#endif /* BULLET_H_ */
