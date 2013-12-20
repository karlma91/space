#include <stdio.h>

#include "states/space.h"

#include "../engine/audio/sound.h"
#include "../engine/graphics/particles.h"
#include "spaceengine.h"

#include "game.h"


/**
 * Collision specific effects
 */
static void add_sparks_at_contactpoint(cpArbiter *arb, float min_force)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	if(cpArbiterGetCount(arb) >0){
		cpVect force = cpArbiterTotalImpulse(arb);
		float f = cpvlength(force) + min_force;
		if (f>50) {
			cpVect v = cpArbiterGetPoint(arb, 0);
			cpVect n = cpArbiterGetNormal(arb, 0);
			float angle = cpvtoangle(n);
			particles_add_sparks(RLAY_GAME_FRONT, v,angle,f);
		}
	}
}

static void se_add_explotion_at_contact_point(cpArbiter *arb)
{
	if(cpArbiterGetCount(arb) >0){
		cpVect v = cpArbiterGetPoint(arb, 0);
		particles_get_emitter_at(RLAY_GAME_FRONT, EMITTER_EXPLOSION, v);
	}
}


/* collision handling */

static void collision_bullet_VS_object_with_score(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	instance *bullet = (instance*)(a->body->data);
	instance *object = (instance *)(b->body->data);

	instance_remove(bullet);

	add_sparks_at_contactpoint(arb, 500);
	//se_add_explotion_at_contact_point(arb);

	//FIXME how to deal with objects already killed?
	if (se_damage_object(object, bullet)) {
		if (object->alive) {
			instance_remove(bullet);
			//se_add_score_and_popup(b->body->p, *COMPONENT(object, SCORE, int*));
		}
	} else {
		if (object->TYPE == obj_id_factory) { //TODO play/get sound_id from object
			sound_play(SND_HIT_1);
		} else {
			sound_play(SND_HIT_2);
		}
	}
	//cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
}
static void collision_bullet_VS_player(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	instance *bullet = (instance*)(a->body->data);
	instance *object = (instance *)(b->body->data);

	instance_remove(bullet);

	if (se_damage_object(object, bullet)) {
		//se_add_explotion_at_contact_point(arb);
	} else {
		sound_play(SND_HIT_2);
	}
}

static void collision_player_object(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	instance *player = ((instance *)(a->body->data));

	if (player)  {
		if (player->TYPE == obj_id_player) {
			add_sparks_at_contactpoint(arb, 0);
			cpVect force = cpArbiterTotalImpulse(arb);
			float f = cpvlength(force);
			if (f > 15) {
				se_damage_deal((instance *)player, f*0.01);
			}
		} else {
			SDL_Log("Expected object type ID %p, but got %p!\n", obj_id_player, player->TYPE);
		}
	} else {
		SDL_Log("Expected object from collision between player and ground, but got NULL\n");
	}
}

static void callback_bullet_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	instance *object = ((instance *)(a->body->data));
	add_sparks_at_contactpoint(arb, 0);
	//sound_play(SND_LASER_MISS);
	instance_remove(object);
}

static void callback_rocket_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	instance *object = ((instance *)(a->body->data));
	//se_add_explotion_at_contact_point(arb);
	instance_remove(object);
}

static void callback_metal_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	add_sparks_at_contactpoint(arb, 0);
}

static int sensor_pickup(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	obj_coin *coin = ((obj_coin *)(b->body->data));

	if (coin) {
		if (coin->data.TYPE != obj_id_coin) {
			SDL_Log("ERROR: expected coin in sensor_pickup!");
			exit(1);
		}
		coin->pulled = 1;
	}
	return 0;
}

void collisioncallbacks_init(void)
{
	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_tank, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_rocket, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_factory, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_robotarm, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_turret, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);

	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, ID_GROUND, NULL, NULL, callback_bullet_ground, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_rocket, ID_GROUND, NULL, NULL, callback_rocket_ground, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_robotarm, ID_GROUND, NULL, NULL, callback_metal_ground, NULL, NULL);

	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_player, NULL, NULL, collision_bullet_VS_player, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_rocket, obj_id_player, NULL, NULL, collision_bullet_VS_player, NULL, NULL);

	cpSpaceAddCollisionHandler(current_space, obj_id_player, ID_GROUND, NULL, NULL, collision_player_object, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_player, obj_id_robotarm, NULL, NULL, collision_player_object, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_player, obj_id_factory, NULL, NULL, collision_player_object, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_player, obj_id_robotarm, NULL, NULL, collision_player_object, NULL, NULL);

	cpSpaceAddCollisionHandler(current_space, obj_id_player, obj_id_coin, sensor_pickup, NULL, NULL, NULL, NULL);
}
