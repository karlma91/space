#include <stdio.h>

#include "obj/objects.h"

#include "states/space.h"

#include "../engine/audio/sound.h"
#include "../engine/graphics/particles.h"
#include "spaceengine.h"

#include "game.h"


static int collision_object_bullet_with_score(cpArbiter *arb, cpSpace *space, void *unused);
static void callback_bullet_ground(cpArbiter *arb, cpSpace *space, void *unused);
static void callback_rocket_ground(cpArbiter *arb, cpSpace *space, void *unused);
static void collision_player_object(cpArbiter *arb, cpSpace *space, void *unused);
static void se_add_explotion_at_contact_point(cpArbiter *arb);
static void add_sparks_at_contactpoint(cpArbiter *arb);
static int collision_object_bullet(cpArbiter *arb, cpSpace *space, void *unused);

void collisioncallbacks_init()
{
	cpSpaceAddCollisionHandler(space, obj_id_tank->ID, obj_id_bullet->ID, collision_object_bullet_with_score, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(space, obj_id_rocket->ID, obj_id_bullet->ID, collision_object_bullet_with_score, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(space, obj_id_factory->ID, obj_id_bullet->ID, collision_object_bullet_with_score, NULL, NULL, NULL, NULL);

	cpSpaceAddCollisionHandler(space, obj_id_bullet->ID, ID_GROUND, NULL, NULL, callback_bullet_ground, NULL, NULL);
	cpSpaceAddCollisionHandler(space, obj_id_bullet->ID, ID_GROUND, NULL, NULL, callback_bullet_ground, NULL, NULL);

	cpSpaceAddCollisionHandler(space, obj_id_player->ID, obj_id_bullet->ID, collision_object_bullet, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(space, obj_id_player->ID, ID_GROUND, NULL, NULL, collision_player_object, NULL, NULL);
	cpSpaceAddCollisionHandler(space, obj_id_player->ID, obj_id_robotarm->ID, NULL, NULL, collision_player_object, NULL, NULL);
	cpSpaceAddCollisionHandler(space, obj_id_player->ID, obj_id_factory->ID, NULL, NULL, collision_player_object, NULL, NULL);

	cpSpaceAddCollisionHandler(space, obj_id_rocket->ID, ID_GROUND, NULL, NULL, callback_rocket_ground, NULL, NULL);
	cpSpaceAddCollisionHandler(space, obj_id_player->ID, obj_id_rocket->ID, collision_object_bullet, NULL, NULL, NULL, NULL);

}

static int collision_object_bullet_with_score(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	instance *object = (instance *)(a->body->data);
	instance *bullet = (instance*)(b->body->data);

	bullet->alive = 0;

	se_add_explotion_at_contact_point(arb);

	//TODO create a function for damaging other objects
	//FIXME how to deal with objects already killed?
	if (se_damage_object(object, *COMPONENT(bullet, DAMAGE, float*))) {
		if (object->alive) {
			object->alive = 0;
			particles_get_emitter_at(EMITTER_EXPLOSION, b->body->p);
			//se_add_score_and_popup(b->body->p, *COMPONENT(object, SCORE, int*));
			sound_play(SND_EXPLOSION);
		}
	} else {
		sound_play(SND_EXPLOSION);
	}
	//cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);

	return 0;
}
static int collision_object_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	instance *object = (instance *)(a->body->data);
	instance *bullet = (instance*)(b->body->data);

	bullet->alive = 0;

	se_add_explotion_at_contact_point(arb);

	if (se_damage_object(object, *COMPONENT(bullet, DAMAGE, float *))) {
		particles_get_emitter_at(EMITTER_EXPLOSION, b->body->p);
		sound_play(SND_EXPLOSION);
	} else {
		sound_play(SND_EXPLOSION);
	}

	return 0;
}

static void collision_player_object(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	instance *player = ((instance *)(a->body->data));

	if (player)  {
		if (player->TYPE->ID == obj_id_player->ID) {
			add_sparks_at_contactpoint(arb);
			cpVect force = cpArbiterTotalImpulse(arb);
			float f = cpvlength(force);
			//todo create a super fancy formula for determining physical damage
			if (f > 10)
				COMPONENT(player, HPBAR, hpbar *)->value -= f * 0.017; // <- changed player force to impulse f * 0.01 // f * 0.05 // 0.033
		} else {
			SDL_Log("Expected object type ID %d, but got %d!\n", obj_id_player->ID, player->TYPE->ID);
		}
	} else {
		SDL_Log("Expected object from collision between player and ground, but got NULL\n");
	}

}

static void callback_bullet_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	instance *object = ((instance *)(a->body->data));
	add_sparks_at_contactpoint(arb);
	sound_play(SND_LASER_1);
	object->alive = 0;
}

static void callback_rocket_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	instance *object = ((instance *)(a->body->data));
	se_add_explotion_at_contact_point(arb);
	object->alive = 0;
}



/**
 * Collision specific effects
 */
static void add_sparks_at_contactpoint(cpArbiter *arb)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	if(cpArbiterGetCount(arb) >0){
		cpVect force = cpArbiterTotalImpulse(arb);
		float f = cpvlength(force);
		if(f>25){
			cpVect v = cpArbiterGetPoint(arb, 0);
			cpVect n = cpArbiterGetNormal(arb, 0);
			float angle = cpvtoangle(n);
			particles_add_sparks(v,angle,f);
		}
	}
}

static void se_add_explotion_at_contact_point(cpArbiter *arb)
{
	if(cpArbiterGetCount(arb) >0){
		cpVect v = cpArbiterGetPoint(arb, 0);
		particles_get_emitter_at(EMITTER_EXPLOSION, v);
	}
}

