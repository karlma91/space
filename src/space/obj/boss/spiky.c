#include "../object_types.h"
#include "../../game.h"
#include "../../states/space.h"
#include "../../spaceengine.h"
#include "chipmunk.h"

#define OBJ_NAME spiky
#include "we_defobj.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	sprite_create(&spiky->data.spr, SPRITE_SPIKEBALL, 400, 400, 30);
	float mass = 10;
	float radius = 750;

	spiky->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetUserData(spiky->data.body, spiky);
	cpBodySetPos(spiky->data.body, spiky->data.p_start);

	cpShape *shape = we_add_circle_shape(current_space, spiky->data.body,radius,0.8,0.2);
	we_shape_collision(shape, &this, LAYER_BUILDING, CP_NO_GROUP);

	sprite_create(&spiky->data.spr, SPRITE_SPIKEBALL, radius*2.5,radius*2.5, 0);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	sprite_render_body(RLAY_GAME_MID, &(spiky->data.spr), spiky->data.body);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &spiky->data.body);
}
