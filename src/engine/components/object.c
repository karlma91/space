#include "object.h"

int object_count = 0;

void object_priv_register(obj_id *obj)
{
	*(int *)(&obj->ID) = object_count;
	++object_count;

	//TODO add into object structure
}



void object_init()
{

}

void object_destroy()
{

}
