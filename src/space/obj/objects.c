#include "objects.h"
#include "../level.h"
#include "../../engine/data/llist.h"
#include "../../engine/io/waffle_utils.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define DEBUG_MEMORY

int LAYER_PLAYER          =     1<<1 | 1<<2 |                1<<5;
int LAYER_TANK            =     1<<1 |         1<<3;
int LAYER_WHEEL           =                                        1<<6;
int LAYER_TANK_FACTORY    =            1<<2  |        1<<4;
int LAYER_PLAYER_BULLET   =                    1<<3 | 1<<4;
int LAYER_ENEMY_BULLET    =                                 1<<5;

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
