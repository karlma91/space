
#include "object_types.h"
#include <stdio.h>

#define OBJ_NAME template
#include "../../engine/components/object.h"

object_p_template template_param = {
	.a=200,
	.b=50
};
static void on_create(OBJ_TYPE *this)
{
	fprintf(stderr, "an instance of %s added!\n", this->ins.TYPE->NAME);

	instance_set_param(this, &template_param);
}

static void on_update(OBJ_TYPE *this)
{
	fprintf(stderr,"template cd: %d\n",this->param.a);

	if (--this->param.a <= 0) {
		instance_remove((instance *)this);
	}

}

static void on_render(OBJ_TYPE *this)
{
}

static void on_destroy(OBJ_TYPE *this)
{
	fprintf(stderr, "one instance of obj_template removed!\n");
}
