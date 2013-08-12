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

/* declaration of all object_id structs */
OBJECT_DECLARE(template);

/* object declarations */
OBJ_STRUCT_START
	/* parameters */
	int a;
	int b;

	float x, y;
} obj_template;


#endif /* OBJECT_TYPES_H_ */
