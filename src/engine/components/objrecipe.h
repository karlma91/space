/*
 * objrecipe.h
 *
 *  Created on: 13. feb. 2014
 *      Author: mathiahw
 */

#ifndef OBJRECIPE_H_
#define OBJRECIPE_H_

#include "we_utils.h"
#include "object.h"
#include "shape.h"
#include "we_graphics.h"
#include "../audio/sound.h"
#include "cJSON.h"
#include "param.h"

typedef struct object_recipe {
	object_id * obj_type;
	char param_name[32];
	void * param;
	cpVect pos;
	cpVect vel;
	float rotation;
}object_recipe;

void objrecipe_init();
void objrecipe_load_objects(LList l, cJSON* object_array, param_list *pl);

#endif /* OBJRECIPE_H_ */
