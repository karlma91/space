#include "object_types.h"


/* object registration declarations */
int add_obj_template();

/* object indexes */
int OBJ_TEMPLATE;

/* registration of all objects */
void object_types_init()
{
	OBJ_TEMPLATE = add_obj_template();


	//TMP TEST
	instance_create(OBJ_TEMPLATE, 0,0, 4,0);
}
