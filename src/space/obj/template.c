#include "object_types.h"

#define OBJ_NAME template
#include "../../engine/components/object.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"


static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	instance_super_free((instance*)OBJ_NAME);
}
