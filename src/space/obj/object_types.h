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

typedef struct {
	instance ins;

	/* OBJECT DATA */
	struct {
		int a;
		int b;
	} var;

	/* PARAMETER DATA */
	void *arg;
} obj_template;

/* */

#endif /* OBJECT_TYPES_H_ */
