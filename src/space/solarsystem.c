#include "solarsystem.h"
#include "game.h"


static void station_free(station *s)
{
	free(s);
}

solarsystem *solarsystem_create(int solsys_index, float star_size, SPRITE_ID star_spr, Color star_base, Color star_glow, Color star_add1, Color star_add2)
{
	solarsystem *solsys = (solarsystem *)calloc(1, sizeof *solsys);
	solsys->index = solsys_index;

	int i = solsys_index + 2;
	cpFloat radius = powf(i, 0.8) * 4800;
	cpFloat angle = WE_2PI * powf(i, 0.8) / 3.768226535;//  + we_randf / 50);
	i = 2;
	cpFloat off_radius = powf(i, 0.8) * 4800;
	cpFloat off_angle = WE_2PI * powf(i, 0.8) / 3.768226535;//  + we_randf / 50);

	cpVect jitter = cpvmult(cpv(we_randf - 0.5, we_randf - 0.5), 100);
	cpVect offset = WE_P2C(off_radius, off_angle);
	solsys->origo = cpvsub(cpvadd(WE_P2C(radius,angle), jitter), offset);

	solsys->station_count = 0;
	solsys->sun.size = star_size;
	solsys->sun.base = star_base;
	solsys->sun.glow = star_glow;
	solsys->sun.add1 = star_add1;
	solsys->sun.add2 = star_add2;
	solsys->sun.pos = solsys->origo;
	solsys->sun.spr_id = star_spr;
	solsys->sun.angvel = 0.03;

	solsys->stations = llist_create();
	llist_set_remove_callback(solsys->stations, station_free);

	return solsys;
}

static void button_callback(void *data)
{
	levelscreen_change_to(data);
}

void solarsystem_add_station(solarsystem * sol, SPRITE_ID spr_id, int dir_type, char * name, char * path)
{
	station *s = calloc(1, sizeof(station));
	strcpy(s->level_path, path);
	int i = llist_size(sol->stations) + 1;
	float size = 300 + we_randf * 100;
	cpFloat radius = (i+2) * 450;
	cpFloat angle = WE_2PI * we_randf;
	s->pos = cpvadd(WE_P2C(radius,angle), sol->origo);
	Color col_back = {255,180,140,255};
	s->btn = button_create(SPRITE_STATION001, 0, s->level_path, s->pos.x, s->pos.y, size, size);
	button_set_click_callback(s->btn, button_callback, s);
	button_set_txt_antirot(s->btn, 1);
	button_set_backcolor(s->btn, col_back);
	button_set_animated(s->btn, 1, (i ? 18 : 15));
	button_set_enlargement(s->btn, 1.5);
	button_set_hotkeys(s->btn, digit2scancode[(i+1) % 10], 0);
	sprite *spr = button_get_sprite(s->btn);
	spr->antirot = 1;
	llist_add(sol->stations,s);
}

void solarsystem_register_touch(solarsystem *sol, STATE_ID id)
{
	llist_begin_loop(sol->stations);
	while (llist_hasnext(sol->stations)) {
		state_register_touchable(id,((station *)llist_next(sol->stations))->btn);
	}
	llist_end_loop(sol->stations);
}

void solarsystem_update(solarsystem *solsys)
{

}

void solarsystem_draw(solarsystem *solsys)
{
	sun_render(RLAY_GAME_BACK, &solsys->sun);
}


void sun_render(int layer, drawbl_sun *sun)
{
	cpVect size = cpv(sun->size,sun->size);
	draw_color(sun->base);
	sprite_render_index_by_id(layer, sun->spr_id, 0, sun->pos, size, 0);
	draw_color(sun->glow);
	sprite_render_index_by_id(layer, SPRITE_GLOW, 0, sun->pos, cpvmult(size,2), 0);
	draw_color(sun->add1);
	sprite_render_index_by_id(layer, sun->spr_id, 1, sun->pos, size, sun->ang1);
	draw_color(sun->add2);
	sprite_render_index_by_id(layer, sun->spr_id, 2, sun->pos, size, sun->ang2);

	float spd = WE_2PI * dt * sun->angvel;
	sun->ang1 += spd;
	sun->ang2 -= spd;
}

void solarsystem_destroy(solarsystem *sy)
{
	llist_destroy(sy->stations);
	free(sy);
}
