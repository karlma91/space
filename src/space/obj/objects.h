#ifndef OBJECTS_H_
#define OBJECTS_H_

#include "chipmunk.h"
#include "../../engine/components/hpbar.h"
#include "../../engine/graphics/sprite.h"

#include "../../engine/components/object.h"

/*
typedef struct { //TODO update for object system V3
	hpbar *hp_bar;
	int *score;
	float *damage;
	int body_count;
	cpBody *(bodies[5]);
} object_component_pointers;
*/

/* COLLISION LAYERS */
extern int LAYER_PLAYER;
extern int LAYER_TANK;
extern int LAYER_WHEEL;
extern int LAYER_TANK_FACTORY;
extern int LAYER_PLAYER_BULLET;
extern int LAYER_ENEMY_BULLET;

extern void instance_nearest_x_two(instance *ins, int obj_id, instance **left, instance **right, cpFloat *instance, cpFloat *right_distance);

#endif /* OBJECTS_H_ */
