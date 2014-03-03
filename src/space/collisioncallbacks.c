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
		particles_get_emitter_at(RLAY_GAME_FRONT, EM_EXPLOSION, v);
	}
}


/* collision handling */

void postStepRemove(cpSpace *space, void *key, meta_tile *meta)
{
	int x = meta->x_col;
	int y = meta->y_row;
	currentlvl->tm.data[TLAY_SOLID][y][x] = TILE_TYPE_NONE;
	cpShape *shape = currentlvl->tm.metadata[y][x].block;
	currentlvl->tm.metadata[y][x].destroyable = WE_FALSE;
	if (shape) {
		fprintf(stderr, "tile is getting removed\n");
		cpSpaceRemoveStaticShape(current_space, shape);
		cpShapeFree(shape);
	}
}

static void collision_bullet_VS_object_with_score(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	instance *bullet = (instance*)(a->body->data);

	instance_remove(bullet);
	add_sparks_at_contactpoint(arb, 500);

	if (b->collision_type == ID_GROUND_DESTROYABLE) {
		float *damage = COMPONENT(bullet, DAMAGE, float*);
		if (damage) {
			fprintf(stderr, "DEBUG: HURTING destroyable tile!\n");
			meta_tile *meta = b->data;
			if (meta->hp > 0 && damage) {
				meta->hp -= *damage;
				if (meta->hp <= 0 && meta->destroyable) {
					fprintf(stderr, "DEBUG: removing destroyable tile!\n");
					cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, b, b->data);
				}
			}
		}
	} else {
		instance *object = (instance *)(b->body->data);

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

static int collision_explosion(cpArbiter *arb, cpSpace *space, void *unused)
{

	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	obj_explosion *expl = ((obj_explosion *)(a->body->data));

	instance *obj = ((instance *)(b->body->data));

	if (se_damage_object(obj, ((instance *)(a->body->data)))) {

	}
	//TODO use param to decide what force to apply
	cpVect f = cpvmult(cpvnormalize_safe(cpvsub(b->body->p, a->body->p)),expl->param.force);
	cpBodyApplyImpulse(b->body, f, cpvzero);

	return 0;
}


void collisioncallbacks_init(void)
{
	cpSpaceAddCollisionHandler(current_space, obj_id_explosion, obj_id_tank, collision_explosion, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_explosion, obj_id_crate, collision_explosion, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_explosion, obj_id_player, collision_explosion, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_explosion, obj_id_robotarm, collision_explosion, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_explosion, obj_id_factory, collision_explosion, NULL, NULL, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_explosion, obj_id_rocket, collision_explosion, NULL, NULL, NULL, NULL);

	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, ID_GROUND_DESTROYABLE, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_tank, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);
	cpSpaceAddCollisionHandler(current_space, obj_id_bullet, obj_id_crate, NULL, NULL, collision_bullet_VS_object_with_score, NULL, NULL);
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
