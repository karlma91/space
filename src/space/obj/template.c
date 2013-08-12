
#include "object_types.h"
#include <stdio.h>

#define OBJ_NAME template
#include "../../engine/components/object.h"


static void on_create(OBJ_TYPE *self)
{
	fprintf(stderr, "one instance of obj_template added!\n");
}

static void on_update(OBJ_TYPE *self)
{
	instance_remove(self);
}

static void on_render(OBJ_TYPE *self)
{
}

static void on_destroy(OBJ_TYPE *self)
{
	fprintf(stderr, "one instance of obj_template removed!\n");
}
