/*
 * upgrades.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

#include "../game.h"
#include "../upgrades.h"
#include "we_defstate.h"

STATE_ID state_store;

static rect box = {{0,0}, {-1,GAME_HEIGHT}};
//static int tex_sketch;

enum UPGRADE_GROUPS {
	UPGRADE_WEAPON,
	UPGRADE_ARMOR,
	UPGRADE_ENGINE,

	UPGRADE_COUNT
};

static button btn_upgrade[UPGRADE_COUNT];
static button btn_next[UPGRADE_COUNT]; //TODO bytte ut knappene med scroll?
static button btn_prev[UPGRADE_COUNT];

static button btn_space;

static const float x_upg = -400;

float upg_total_mass = 0;
static float dps = 0;

//TODO place upgrade indexes in player struct?
int weapon_index = 0;
int armor_index = 0;
int engine_index = 0;

static int weapon_selected = 0;
static int armor_selected = 0;
static int engine_selected = 0;

static void update_upgrades(void);

SDL_Scancode digit2scancode[10] = {
		SDL_SCANCODE_0,
		SDL_SCANCODE_1,
		SDL_SCANCODE_2,
		SDL_SCANCODE_3,
		SDL_SCANCODE_4,
		SDL_SCANCODE_5,
		SDL_SCANCODE_6,
		SDL_SCANCODE_7,
		SDL_SCANCODE_8,
		SDL_SCANCODE_9,
};


/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
	update_upgrades();
}

static void pre_update(void)
{
}

static void post_update(void)
{
}

static void draw_testarea(void)
{
	//glScissor(300,200,500,500);

	draw_color4f(0,0,0,1);
	draw_load_identity();
	draw_box(2, cpv(350,180),cpv(550,550),0,1);


	/*TODO gj�re om test area til en egen state? som viser player som kj�rer og skyter
	 * mens man er i store state, og gj�re slik at man kan g� inn i test state for � pr�ve
	 * skipet.
	 */

	//glScissor(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
}

static void draw_specs(void)
{
	static const float specs_x = 100;
	draw_color4f(1,1,1,1);
	setTextAlign(TEXT_LEFT);
	setTextSize(24);

	upg_weapon *w = &weapons[weapon_selected];
	upg_armor *a = &armors[armor_selected];
	upg_engine *e = &engines[engine_selected];

	char spec_buffer[250];

	sprintf(&spec_buffer[0], "WEAPON LEVEL %d\nDAMAGE: %6.1f\nFIRERATE: %4.1f\nDPS: %9.1f\n\nHP: %7.0f\nMASS: %5.0f\nFORCE: %4.0f\nSPEED: %4.0f",
			w->level+1, w->lvls[w->level].damage, w->lvls[w->level].firerate,dps, a->max_hp, upg_total_mass, e->force, e->max_speed);
	font_drawText(specs_x, -200, spec_buffer);
}

static void draw(void)
{
	float y;
	float y_offset = 140;
	draw_color4f(0.1,0.2,0.4,1);
	draw_box(4, box.p,box.s,0,1);

	//draw_color4f(1,1,1,1);
	//draw_texture(tex_sketch, &cpvzero,TEX_MAP_FULL, box.w,box.h,0);

	draw_color4f(1,1,1,1);
	setTextAlign(TEXT_CENTER);

	setTextSize(20);
	y = btn_next[UPGRADE_WEAPON]->get.y;
	font_drawText(x_upg,y + y_offset,weapons[weapon_selected].name);
	font_draw_int(x_upg,y - y_offset,weapons[weapon_selected].lvls[weapons[weapon_selected].level].price);

	y = btn_next[UPGRADE_ARMOR]->get.y;
	font_drawText(x_upg,y + y_offset,armors[armor_selected].name);
	font_draw_int(x_upg,y - y_offset,armors[armor_selected].price);

	y = btn_next[UPGRADE_ENGINE]->get.y;
	font_drawText(x_upg,y + y_offset,engines[engine_selected].name);
	font_draw_int(x_upg,y - y_offset,engines[engine_selected].price);

	setTextSize(50);
	font_drawText(0, box.p.y+box.s.y / 2 - 60, "STORE");

	draw_testarea();
	draw_specs();
}

static void sdl_event(SDL_Event *event)
{
}

static void on_pause(void)
{
}

static void on_leave(void)
{
}

static void destroy(void)
{
}


static void update_upgrades(void)
{
	upg_weapon *w = &weapons[weapon_selected];
	upg_armor *a = &armors[armor_selected];
	upg_engine *e = &engines[engine_selected];

	upg_total_mass = w->lvls[w->level].mass + a->mass;
	dps = w->lvls[w->level].damage * w->lvls[w->level].firerate;

	button_set_sprite(btn_upgrade[UPGRADE_WEAPON], *(w->spr_id));
	button_set_sprite(btn_upgrade[UPGRADE_ARMOR], *(a->spr_id));
	button_set_sprite(btn_upgrade[UPGRADE_ENGINE], *(e->spr_id));

	//TODO TMP code
	weapon_index = weapon_selected;
	armor_index = armor_selected;
	engine_index = engine_selected;
}

static void next_upgrade(enum UPGRADE_GROUPS id)
{
	switch(id) {
	case UPGRADE_WEAPON:
		++weapon_selected;
		weapon_selected = (weapon_selected >= WEAPON_UPGRADES) ? 0 : weapon_selected;
		break;
	case UPGRADE_ARMOR:
		++armor_selected;
		armor_selected = (armor_selected >= ARMOR_UPGRADES) ? 0 : armor_selected;
		break;
	case UPGRADE_ENGINE:
		++engine_selected;
		engine_selected = (engine_selected >= ENGINE_UPGRADES) ? 0 : engine_selected;
		break;
	default:
		/* invalid value */
		break;
	}
	update_upgrades();
}

static void prev_upgrade(enum UPGRADE_GROUPS id)
{
	switch(id) {
	case UPGRADE_WEAPON:
		--weapon_selected;
		weapon_selected = (weapon_selected < 0) ? WEAPON_UPGRADES - 1 : weapon_selected;
		break;
	case UPGRADE_ARMOR:
		--armor_selected;
		armor_selected = (armor_selected < 0) ? ARMOR_UPGRADES - 1 : armor_selected;
		break;
	case UPGRADE_ENGINE:
		--engine_selected;
		engine_selected = (engine_selected < 0) ? ENGINE_UPGRADES - 1 : engine_selected;
		break;
	default:
		/* invalid value */
		break;
	}
	update_upgrades();
}

static void upgrade_click(enum UPGRADE_GROUPS id)
{
	//TODO create confirmation dialog box, asking if user want to buy upgrade?
	switch(id) {
	case UPGRADE_WEAPON:
		//TODO remove TMP test code!
		if (++weapons[weapon_selected].level > 2) {
			weapons[weapon_selected].level = 0;
		}
		break;
	case UPGRADE_ARMOR:
		break;
	case UPGRADE_ENGINE:
		break;
	default:
		/* invalid value */
		break;
	}
	update_upgrades();
}

void upgrades_init(void)
{
	statesystem_register(state_store,0);

	box.s.x = GAME_WIDTH;

	float x_prev = x_upg - 255;
	float x_next = x_upg + 255;

	int i;
	for (i = 0; i < UPGRADE_COUNT; i++) {
		float y = 300 - i*330;
		btn_next[i] = button_create(SPRITE_GEAR, 0, "", x_next, y, 230, 230);
		button_set_callback(btn_next[i], (btn_callback)next_upgrade, i);
		button_set_enlargement(btn_next[i], 1.5);
		state_register_touchable(this, btn_next[i]);

		btn_prev[i] = button_create(SPRITE_GEAR, 0, "", x_prev, y, 230, 230);
		button_set_callback(btn_prev[i], (btn_callback)prev_upgrade, i);
		button_set_enlargement(btn_prev[i], 1.5);
		state_register_touchable(this, btn_prev[i]);

		btn_upgrade[i] = button_create(NULL, 0, "", x_upg, y, 230, 230);
		button_set_callback(btn_upgrade[i], (btn_callback)upgrade_click, i);
		button_set_enlargement(btn_upgrade[i], 1.5);
		state_register_touchable(this, btn_upgrade[i]);
	}

	btn_space = button_create(SPRITE_HOME, 0, "", -GAME_WIDTH/2 + 100, GAME_HEIGHT/2 - 100, 130, 130);
	button_set_callback(btn_space, statesystem_set_state, state_stations);
	button_set_enlargement(btn_space, 1.5);
	button_set_hotkeys(btn_space, KEY_ESCAPE, 0);
	state_register_touchable(this, btn_space);

	//TODO load balance and bought items/upgrades
	for (i = 0; i < WEAPON_UPGRADES; i++) {
		weapons[i].level = 1; //TMP inntil loading er laget
	}

	update_upgrades();
}

