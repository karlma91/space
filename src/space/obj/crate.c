
#include "object_types.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define OBJ_NAME crate
#include "we_defobj.h"
#include "chipmunk_unsafe.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
	cpBodySetPos(crate->data.body, crate->data.p_start);
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(crate, HPBAR, &crate->hp_bar);
	COMPONENT_SET(crate, COINS, &crate->param.coins);
	sprite_create(&(crate->data.spr), crate->param.sprite, crate->param.size, crate->param.size, 0);
	float mass = 1;
	cpFloat moment = cpMomentForBox(mass, crate->param.size, crate->param.size);
	crate->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, moment));
	cpBodySetPos(crate->data.body, crate->data.p_start); //FIXME
	cpBodySetUserData(crate->data.body, (instance*) crate);
	se_velfunc(crate->data.body, 1);

	cpShape *shape = we_add_box_shape(current_space, crate->data.body, crate->param.size, crate->param.size, 0.9, 0.5);
	cpShapeSetCollisionType(shape, &this);
	cpShapeSetGroup(shape, crate);

	hpbar_init(&(crate->hp_bar), crate->param.max_hp, 100, 25, 0, 50, &(crate->data.body->p));
	hpbar_set_invincible(&(crate->hp_bar), crate->param.invinc);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{

}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color(COL_WHITE);
	float border = 40;
	draw_quad_patch_center_spr(RLAY_GAME_MID, &(crate->data.spr),crate->data.body->p,cpv(crate->param.size-border*2, crate->param.size-border*2), border, crate->data.body->a);
	hpbar_draw(RLAY_GUI_BACK, &crate->hp_bar, cpvtoangle(crate->data.body->p));
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	explosion_create(crate->data.body->p, EM_EXPLOSIONBIG, EM_FRAGMENTS, SND_BUILDING_EXPLODE, 2500, 250, 0.3, 4);
	se_spawn_coins((instance *)crate);
	instance_remove((instance *)crate);
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &crate->data.body);
}
