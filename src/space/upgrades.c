/*
 * upgrades.c
 *
 *  Created on: 27. juli 2013
 *      Author: Mathias
 */
//TODO USE JSON FORMAT TO STORE UPGRADES INSTEAD
#include "upgrades.h"
#include "game.h"

upg_weapon weapons[WEAPON_UPGRADES] = {
	{
		.name = "ECO LASER",
		.p_type = PROJECT_LASER,
		.level = 1,
		.lvls = {
			/* dmg, rate, mass, range, price */
			{60, 10.5, 2.0, 600, 0}, //58
			{25, 20.4, 3.5, 620, 3200}, // 64
			{17, 30.7, 4.0, 650, 4500}  // 78
		},
		.spr_id = &SPRITE_PLAYERGUN002,
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
	{.name = "MIDDLE ARMOR", .max_hp = 500, .mass = 2, .shield = 0, .shield_regen = 0, .price = 13200, .spr_id = &SPRITE_PLAYERBODY002},
	{.name = "ELEPHANT ARMOR", .max_hp = 800, .mass = 3, .shield = 0, .shield_regen = 0, .price = 13200, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "T-REX SKIN", .max_hp = 100000, .mass = 10, .shield = 0, .shield_regen = 0, .price = 9999990, .spr_id = &SPRITE_SPIKEBALL}
};

upg_engine engines[ENGINE_UPGRADES] = {
	{.name = "TURTLE POWER", .force = 120, .max_speed = 435, .price = 0, .particle_type = &EM_FLAME, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "CHEAP JET", .force = 150, .max_speed = 475, .price = 9500, .particle_type = &EM_FLAME, .spr_id = &SPRITE_PLAYERBODY001},
	{.name = "Jetlag", .force = 200, .max_speed = 600, .price = 99950, .particle_type = &EM_FLAME, .spr_id = &SPRITE_STATION001},
	{.name = "UBERSPEED", .force = 400, .max_speed = 700, .price = 999500, .particle_type = &EM_FLAME, .spr_id = &SPRITE_STATION001}
};

/*
 * (Be able to upgrade indivudal motors?)
 * Try not to make upgrades obsolote
 *
 * need to balance acceleration vs. max speed vs. usabillity
 * 1: lowest
 * 2: low
 * 3: mid
 * 4: high
 * 5: highest
 * cheap category:
 * a v
 * 3 1
 * 3 2
 * 2 3
 *
 * Cheap:     high a == low vmax (Better control + maneuverability)
 * Mid:       mid a == mid vmax (faster + ok maneuverability)
 * cheap: low a == high vmax (faster + ok maneuverability)
 *
 */

void test_me(void)
{
	/*
	upg_weapon w;
	w.lvls[w.level].damage = 100;
	*/
}
