#ifndef BULLET_H_
#define BULLET_H_

#include "chipmunk.h"
#include "draw.h"
#include "objects.h"
#include "particles.h"
#include "space.h"

object *bullet_init(cpVect pos, cpVect dir, int type);

extern struct obj_type type_bullet;

struct bullet{
	/*object*/
	struct obj_type *type;
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
