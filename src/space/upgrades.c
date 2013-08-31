/*
 * upgrades.c
 *
 *  Created on: 27. juli 2013
 *      Author: Mathias
 */

#include "upgrades.h"
#include "game.h"

upg_weapon weapons[WEAPON_UPGRADES] = {
	{
		.name = "ECO LASER",
		.p_type = PROJECT_LASER,
		.level = 1,
		.lvls = {
			/* dmg, rate, mass, range, price */
			{28, 3.1, 1.0, 600, 0},
			{34, 4.4, 1.5, 620, 3200},
			{48, 5.7, 2.0, 650, 4500}
		},
		.spr_id = &SPRITE_PLAYER_GUN,
		.obj_name = "BULLET"
	}, {
		.name = "MACHINE GUN",
		.p_type = PROJECT_BULLET,
		.level = 1,
		.lvls = {
			/* dmg, rate, mass, range, price */
			{7.6, 18.0, 1.8, 550, 12900},
			{8.4, 23.0, 2.3, 560, 4900},
			{9.4, 26.0, 2.6, 570, 5950}
		},
		.spr_id = &SPRITE_PLAYER_GUN,
		.obj_name = "BULLET"
	}, {
			.name = "COIN DROPPER",
			.p_type = PROJECT_BULLET,
			.level = 1,
			.lvls = {
				/* dmg, rate, mass, range, price */
				{0, 5.0, 0, 99999, 999900},
				{0, 15.0, 0, 99999, 999992},
				{0, 30.0, 0, 99999, 999998}
			},
			.spr_id = &SPRITE_SPIKEBALL,
			.obj_name = "COIN"
		}
};

upg_armor armors[ARMOR_UPGRADES] = {
	{.name = "BASIC ARMOR", .max_hp = 200, .mass = 1, .shield = 0, .shield_regen = 0, .price = 0, .spr_id = &SPRITE_PLAYER},
	{.name = "ELEPHANT ARMOR", .max_hp = 750, .mass = 3, .shield = 0, .shield_regen = 0, .price = 13200, .spr_id = &SPRITE_PLAYER}
};

upg_engine engines[ENGINE_UPGRADES] = {
	{.name = "TURTLE POWER", .force = 160, .max_speed = 435, .price = 0, .particle_type = &EMITTER_FLAME, .spr_id = &SPRITE_PLAYER},
	{.name = "LIGHTNING JET", .force = 550, .max_speed = 475, .price = 9500, .particle_type = &EMITTER_FLAME, .spr_id = &SPRITE_PLAYER}
};

void test_me(void)
{
	/*
	upg_weapon w;
	w.lvls[w.level].damage = 100;
	*/
}
