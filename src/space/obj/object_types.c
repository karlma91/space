#include "object_types.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "../level.h"
#include "../../engine/data/llist.h"

/* Bit-layers */
#define L1 (1 << 0)
#define L2 (1 << 1)
#define L3 (1 << 2)
#define L4 (1 << 3)
#define L5 (1 << 4)
#define L6 (1 << 5)
#define L7 (1 << 6)
#define L8 (1 << 7)

/* Collision Layer Setup */
int LAYER_PLAYER        = L1 | L2 | L3 | L4;
int LAYER_ENEMY         = L1 | L5;
int LAYER_BUILDING      = L2 | L6 | L7 | L8;
int LAYER_BULLET_PLAYER = L5 | L6;
int LAYER_BULLET_ENEMY  = L3;
int LAYER_SHIELD_PLAYER = 0;
int LAYER_SHIELD_ENEMY  = 0;
int LAYER_PICKUP        = L4 | L8;

/* registration of all objects */
void object_types_init()
{
	OBJECT_REGISTER(template);

	OBJECT_REGISTER(bullet);
	OBJECT_REGISTER(player);
	OBJECT_REGISTER(factory);
	OBJECT_REGISTER(tank);
	OBJECT_REGISTER(turret);
	OBJECT_REGISTER(rocket);
	OBJECT_REGISTER(robotarm);
	OBJECT_REGISTER(spikeball);

	OBJECT_REGISTER(coin);
}



#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

void instance_nearest_x_two(instance *object, object_id *obj_id, instance **left, instance **right, cpFloat *left_distance, cpFloat *right_distance)
{
	LList list = object_get_instances(obj_id);

	cpFloat min_length_left = INT_MIN;
	cpFloat min_length_right = INT_MAX;
	cpFloat length;

	int check_id = (obj_id == object->TYPE);

	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		instance *obj = (instance *) llist_next(list);
		if (check_id && object->instance_id == obj->instance_id)
			continue; //skip self

		cpVect v = obj->body->p;
		length = v.x - object->body->p.x;
		if (length < 0) { //left side
			if (length > min_length_left) {
				min_length_left = length;
				*left = obj;
			}
		} else { //right side
			if (length < min_length_right) {
				min_length_right = length;
				*right = obj;
			}
		}
	}
	llist_end_loop(list);


	if (min_length_left == INT_MIN) {
		*left = NULL;
		*left_distance =  INT_MAX;
	} else {
		*left_distance =  -min_length_left;
	}
	if (min_length_right == INT_MAX) {
		*right = NULL;
		*right_distance = INT_MAX;
	} else {
		*right_distance = min_length_right;
	}
}
