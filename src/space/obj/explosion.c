
#include "object_types.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define OBJ_NAME explosion
#include "we_defobj.h"


static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	//explosion;
	//particles_get_emitter_at(current_particles, RLAY_GAME_FRONT, EMITTER_EXPLOSION_BIG, );
	sound_play(SND_FACTORY_EXPLODE);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
}
