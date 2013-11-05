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
			{50, 1.5, 1.0, 600, 0}, //58
			{25, 2.4, 1.5, 620, 3200}, // 64
			{17, 3.7, 2.0, 650, 4500}  // 78
		},
		.spr_id = &SPRITE_PLAYERGUN001,
		.obj_name = "BULLET"
	}, {
		.name = "MACHINE GUN",
		.p_type = PROJECT_BULLET,
		.level = 1,
		.lvls = {
			/* dmg, rate, mass, range, price */
			{7.5, 18.0, 1.8, 550, 12900}, // 7.6
			{5.0, 23.0, 2.3, 560, 4900},  // 8.4
			{3.0, 26.0, 2.6, 570, 5950}   // 9.4
		},
		.spr_id = &SPRITE_PLAYERGUN001,
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
	{.name = "BASIC ARMOR", .max_hp = 200, .mass = 1, .shield = 0, .shield_regen = 0, .price = 0, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "MIDDLE ARMOR", .max_hp = 500, .mass = 2, .shield = 0, .shield_regen = 0, .price = 13200, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "ELEPHANT ARMOR", .max_hp = 800, .mass = 3, .shield = 0, .shield_regen = 0, .price = 13200, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "T-REX SKIN", .max_hp = 100000, .mass = 10, .shield = 0, .shield_regen = 0, .price = 9999990, .spr_id = &SPRITE_SPIKEBALL}
};

upg_engine engines[ENGINE_UPGRADES] = {
	{.name = "TURTLE POWER", .force = 80, .max_speed = 435, .price = 0, .particle_type = &EMITTER_FLAME, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "CHEAP JET", .force = 160, .max_speed = 475, .price = 9500, .particle_type = &EMITTER_FLAME, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "UBERSPEED", .force = 500, .max_speed = 9999, .price = 999500, .particle_type = &EMITTER_FLAME, .spr_id = &SPRITE_STATION001}
};

void test_me(void)
{
	/*
	upg_weapon w;
	w.lvls[w.level].damage = 100;
	*/
}
