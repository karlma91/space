/*
 * upgrades.h
 *
 *  Created on: 28. juli 2013
 *      Author: Mathias
 */

#ifndef UPGRADES_H_
#define UPGRADES_H_

#include "../engine/graphics/sprite.h"

typedef enum {
	PROJECT_LASER,
	PROJECT_MULTI_LASER, /* shoots in multiple directions at the same time */
	PROJECT_BULLET, /* e.g. machine gun type of weapon (but more affected by gravity?) */
	PROJECT_MULTI_BULLET,
	PROJECT_ROCKET,  /* e.g. enemey-seeking missiles, unguided rockets */
	PROJECT_EXPLOSIVE
} PROJECTILE_TYPE;

#define MAX_WEAPON_LEVEL 3
#define WEAPON_UPGRADES 3
#define ARMOR_UPGRADES 3
#define ENGINE_UPGRADES 3

typedef struct {
	char name[50];
	PROJECTILE_TYPE p_type; // bruke p_type for � bestemme hvilket objekt som skal opprettes
	int level; /* weapon level[0-2], each lvl contributes to minor enhancements */

	//float bullets_per_shot //TODO gj�re slik at enkelte v�pene kan skyte flere skudd samtidig (kan evt. bare bestemmes ut i fra skytemetoden)
	struct {
		float damage;   /* amount of dmg each bullet deals */
		float firerate; /* shots per second */
		float mass;
		float range;
		//float magazine_size;   /* number of shots per round */
		//float magazine_reload; /* reload/cooldown time in seconds */

		int price; // lvls[0].price == price of weapon. lvls[1].price == upgrade price lvl 1 -> lvl 2, etc...

		//SPRITE_ID spr_id; //TODO have an individual sprite for each weapon level?
	} lvls[MAX_WEAPON_LEVEL];

	SPRITE_ID *spr_id;

	char obj_name[40]; //TODO change to pointer to shoot function with player as argument (or keep both and use as argument)
} upg_weapon;

typedef struct {
	char name[50];
	float max_hp;
	float mass;

	float shield;
	float shield_regen;

	int price;

	SPRITE_ID *spr_id;
} upg_armor;

typedef struct {
	char name[50];
	float force;
	float max_speed;

	int price;

	int *particle_type; //or rather store particle file name?

	SPRITE_ID *spr_id;
} upg_engine;

extern upg_weapon weapons[WEAPON_UPGRADES];
extern upg_armor armors[ARMOR_UPGRADES];
extern upg_engine engines[ENGINE_UPGRADES];

#endif /* UPGRADES_H_ */
