#include "collisioncallbacks.h"
#include "objects.h"
#include "spaceengine.h"
#include "particles.h"
#include "game.h"
#include "space.h"
#include <stdio.h>
#include "sound.h"
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
	cpSpaceAddCollisionHandler(space, ID_TANK, ID_BULLET_PLAYER, collision_object_bullet_with_score, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(space, ID_ROCKET, ID_BULLET_PLAYER, collision_object_bullet_with_score, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(space, ID_FACTORY, ID_BULLET_PLAYER, collision_object_bullet_with_score, NULL, NULL, NULL, NULL);

	cpSpaceAddCollisionHandler(space, ID_BULLET_PLAYER, ID_GROUND, NULL, NULL, callback_bullet_ground, NULL, NULL);
	cpSpaceAddCollisionHandler(space, ID_BULLET_ENEMY, ID_GROUND, NULL, NULL, callback_bullet_ground, NULL, NULL);

	cpSpaceAddCollisionHandler(space, ID_PLAYER, ID_BULLET_ENEMY, collision_object_bullet, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(space, ID_PLAYER, ID_GROUND, NULL, NULL, collision_player_object, NULL, NULL);
	cpSpaceAddCollisionHandler(space, ID_PLAYER, ID_FACTORY, NULL, NULL, collision_player_object, NULL, NULL);

	cpSpaceAddCollisionHandler(space, ID_ROCKET, ID_GROUND, NULL, NULL, callback_rocket_ground, NULL, NULL);
	cpSpaceAddCollisionHandler(space, ID_PLAYER, ID_ROCKET, collision_object_bullet, NULL, NULL, NULL, NULL);

}

static int collision_object_bullet_with_score(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	object_data *object = (object_data *)(a->body->data);
	object_data *bullet = (object_data*)(b->body->data);

	bullet->alive = 0;

	se_add_explotion_at_contact_point(arb);

	//TODO create a function for damaging other objects
	//FIXME how to deal with objects already killed?
	if (se_damage_object(object, *(bullet->components.damage))) {
		if (object->alive) {
			object->alive = 0;
			particles_get_emitter_at(EMITTER_EXPLOSION, b->body->p);
			se_add_score_and_popup(b->body->p, *(object->components.score));
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
	object_data *object = (object_data *)(a->body->data);
	object_data *bullet = (object_data*)(b->body->data);

	bullet->alive = 0;

	se_add_explotion_at_contact_point(arb);

	if (se_damage_object(object, *(bullet->components.damage))) {
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
	object_data *player = ((object_data *)(a->body->data));

	if (player)  {
		if (player->preset->ID == ID_PLAYER) {
			add_sparks_at_contactpoint(arb);
			cpVect force = cpArbiterTotalImpulse(arb);
			float f = cpvlength(force);
			//todo create a super fancy formula for determining physical damage
			if (f > 10)
				player->components.hp_bar->value -= f * 0.033; // <- changed player force to impulse f * 0.01 // f * 0.05
		} else {
			SDL_Log("Expected object type ID %d, but got %d!\n", ID_PLAYER, player->preset->ID);
		}
	} else {
		SDL_Log("Expected object from collision between player and ground, but got NULL\n");
	}

}

static void callback_bullet_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	object_data *object = ((object_data *)(a->body->data));
	add_sparks_at_contactpoint(arb);
	sound_play(SND_LASER_1);
	object->alive = 0;
}

static void callback_rocket_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	object_data *object = ((object_data *)(a->body->data));
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

