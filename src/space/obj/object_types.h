/*
 * object_types.h
 *
 *  Created on: 8. aug. 2013
 *      Author: Mathias
 */

#ifndef OBJECT_TYPES_H_
#define OBJECT_TYPES_H_

#include "../game.h"

void object_types_init(void);
void instance_get2nearest(instance *ins, object_id *obj_id, instance **left, instance **right, cpFloat *instance, cpFloat *right_distance);
void factory_remove_child(instance *child);
void explosion_create(cpVect p, EMITTER_ID em_expl, EMITTER_ID em_frag, Mix_Chunk *snd, float force, float size, float duration, float dmg);

/* COLLISION LAYERS */
extern int LAYER_PLAYER;
extern int LAYER_ENEMY;
extern int LAYER_BUILDING;
extern int LAYER_BULLET_PLAYER;
extern int LAYER_BULLET_ENEMY;
extern int LAYER_SHIELD_PLAYER;
extern int LAYER_SHIELD_ENEMY;
extern int LAYER_PICKUP;


#endif /* OBJECT_TYPES_H_ */
