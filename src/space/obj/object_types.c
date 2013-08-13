#include "object_types.h"

#include <stdio.h>

/* registration of all objects */
void object_types_init()
{
	OBJECT_REGISTER(template);

	//TMP TEST
	instance_create(obj_id_template, NULL, 0,0, 4,0);
}
