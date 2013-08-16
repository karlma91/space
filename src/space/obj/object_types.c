#include "object_types.h"

#include <stdio.h>

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
}
