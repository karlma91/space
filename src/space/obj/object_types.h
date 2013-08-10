/*
 * object_types.h
 *
 *  Created on: 8. aug. 2013
 *      Author: Mathias
 */

#ifndef OBJECT_TYPES_H_
#define OBJECT_TYPES_H_

#include "../../engine/components/object.h"

void object_types_init();

/* object declarations */
OBJ_STRUCT
	/* parameters */
	int a;
	int b;

	float x, y;
} obj_template;


int test() {
	obj_template tmp;

	return 0;
}

#endif /* OBJECT_TYPES_H_ */
