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

/* OBJECT DECLARATION */
OBJECT_DECLARE(template);


/* OBJECT DEFINITIONS */
PARAM_START(template) int a; int b; PARAM_END
OBJ_START(template) float x, y; OBJ_END


#endif /* OBJECT_TYPES_H_ */
