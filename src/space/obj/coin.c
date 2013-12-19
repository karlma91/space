#include "object_types.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define OBJ_NAME coin
#include "we_defobj.h"

#define COIN_DAMPING 0.90f
#define COIN_RADIUS 20
#define COIN_SIZE 40
#define COIN_MASS 0.1f
#define COIN_FPS 15

#define COIN_TIME_ALIVE 10.0
#define COIN_TIME_FADEOUT 3.0

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void vel_func(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
	cpBodyUpdateVelocity(body, cpvzero, COIN_DAMPING, dt);
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	sprite_create(&coin->data.spr, SPRITE_COIN, COIN_SIZE, COIN_SIZE, COIN_FPS);
	sprite_set_index(&coin->data.spr, rand() & 0xF);

	coin->data.body = cpSpaceAddBody(current_space, cpBodyNew(COIN_MASS, cpMomentForCircle(COIN_MASS, 0.0f, COIN_RADIUS, cpvzero)));
	cpBodySetUserData(coin->data.body, coin);
	cpBodySetPos(coin->data.body, coin->data.p_start);
	se_tangent_body(coin->data.body);

	float angle = we_randf * WE_2PI;
	float force = we_randf * coin->param.explo_fmax;
	coin->data.body->v = cpvmult(cpvforangle(angle), force);

	cpShape *shape = we_add_circle_shape(current_space, coin->data.body,COIN_RADIUS-5,0.8,0.2);
	we_shape_collision(shape, &this, LAYER_PICKUP, &this);

	coin->data.body->velocity_func = vel_func;
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	sprite_update(&(coin->data.spr));
	se_tangent_body(coin->data.body);

	if (coin->pulled) {
		instance *player = instance_nearest(coin->data.body->p, obj_id_player);

		//TODO handle TMP magnet
		cpVect diff = se_dist_a2b((instance *)coin, player);
		float length = cpvlength(diff);
			//coin->pulled = 0;
			//diff = cpvmult(diff, 50/length);
		if (length > 60) {
			diff = cpvnormalize(diff);
			diff = cpvmult(diff, 8400*dt);
			coin->data.body->v = cpvadd(coin->data.body->v, diff);
		} else {
			((obj_player *) player)->coins += 10;
			sound_play(SND_COIN);
			instance_remove((instance *)coin);
		}
	} else if (coin->data.time_alive > COIN_TIME_ALIVE) {
		instance_destroy((instance *)coin);
	}
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	byte alpha = 255;
	float time = coin->data.time_alive - (COIN_TIME_ALIVE - COIN_TIME_FADEOUT);
	if (time > 0) {
		alpha = 255 * (COIN_TIME_FADEOUT - time) / COIN_TIME_FADEOUT;
	}
	draw_color_rgbmulta4b(255,255,255,alpha);
	sprite_render_body(RLAY_GAME_FRONT, &(coin->data.spr), coin->data.body);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	instance_remove((instance *)coin);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &coin->data.body);
}
