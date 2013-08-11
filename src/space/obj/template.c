
#include "object_types.h"

#define OBJ_TYPE_NAME obj_template
#include "../../engine/components/object.h"

int add_obj_template()
{
	return REGISTER_OBJ();
}


static void on_create(instance *self)
{
	fprintf(stderr, "one instance of obj_template added!\n");
}

static void on_update(instance *self)
{
	instance_remove(self);
}

static void on_render(instance *self)
{
}

static void on_destroy(instance *self)
{
	fprintf(stderr, "one instance of obj_template removed!\n");
}
