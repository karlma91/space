#include "object_types.h"

#include <stdio.h>

/* registration of all objects */
void object_types_init()
{
	OBJECT_REGISTER(template);

	OBJECT_REGISTER(player);
	OBJECT_REGISTER(bullet);
	OBJECT_REGISTER(tank);
	OBJECT_REGISTER(factory);
	OBJECT_REGISTER(turret);
	OBJECT_REGISTER(rocket);

	//TMP TEST
	instance_create(obj_id_template, NULL, 0,0, 4,0);
}
