#include "object.h"

int object_count = 0;

void object_priv_register(obj_id *obj)
{
	*(int *)(&obj->ID) = object_count;
	++object_count;

	//TODO add into object structure
}

instance *instance_create(obj_id id, cpVect pos, cpVect speed)
{
	//TODO register in object list
}

void object_init()
{

}

void object_destroy()
{

}
