#include "solarsystem.h"
#include "game.h"
#include "states/levelscreen.h"

static void station_free(station *s)
{
	free(s);
}

void solarsystem_create_drawbl(drawbl_sun * sun, float star_size, SPRITE_ID star_spr, Color star_base, Color star_glow, Color star_add1, Color star_add2)
{
	sun->size = star_size;
	sun->base = star_base;
	sun->glow = star_glow;
	sun->add1 = star_add1;
	sun->add2 = star_add2;
	sun->spr_id = star_spr;
	sun->angvel = 0.03;
}

solarsystem *solarsystem_create(int solsys_index, char *name, char *folder)
{
	solarsystem *solsys = (solarsystem *)calloc(1, sizeof *solsys);
	solsys->index = solsys_index;

	strcpy(solsys->name, name);
	strcpy(solsys->folder, folder);

	int i = solsys_index + 2;
	cpFloat radius = powf(i, 0.8) * 4800;
	cpFloat angle = WE_2PI * powf(i, 0.8) / 3.768226535;//  + we_randf / 50);
	i = 2;
	cpFloat off_radius = powf(i, 0.8) * 4800;
	cpFloat off_angle = WE_2PI * powf(i, 0.8) / 3.768226535;//  + we_randf / 50);

	cpVect jitter = cpvmult(cpv(we_randf - 0.5, we_randf - 0.5), 100);
	cpVect offset = WE_P2C(off_radius, off_angle);
	solsys->origo = cpvsub(cpvadd(WE_P2C(radius,angle), jitter), offset);

	solsys->sun.pos = solsys->origo;
	solsys->sun.angvel = 0.03;

	SPRITE_ID spr_sun = sprite_link("sun01");
	float rnd = rand() & 0x1f;
	Color base = {0x70-rnd,0x30,0x30+rnd,0xff};
	Color glow = {0xff-rnd,0xa0,0x70+rnd,0x80};
	Color add1 = {0x90-rnd,0x80,0x40+rnd,0x00};
	Color add2 = {0xb0-rnd,0x70,0x40+rnd,0x00};
	int size = (500 + we_randf*(solsys_index*1500/2 + (solsys_index>1?1000:0)) + 300*solsys_index/2);
	solarsystem_create_drawbl(&(solsys->sun),size, spr_sun,base,glow,add1,add2);
	solsys->stations = llist_create();
	return solsys;
}

static void button_callback(void *data)
{
	levelscreen_change_to(data);
}

void solarsystem_add_station(solarsystem * sol, SPRITE_ID spr_id, Color color, int dir_type, char * name, char * path, char * author, char *filename)
{
	station *s = calloc(1, sizeof(station));
	s->sol = sol;
	strcpy(s->path, path);
	strcpy(s->name, name);
	strcpy(s->author, author);
	strcpy(s->filename, filename);
	int i = llist_size(sol->stations);
	float size = 300 + we_randf * 100;
	cpFloat radius = sol->sun.size + (i) * 400 ;
	cpFloat angle = WE_2PI * we_randf;
	s->radius = radius;
	s->angle = angle;
	s->rotation_speed = 1000/radius;
	s->pos = cpvadd(WE_P2C(radius,angle), sol->origo);
	s->col = color;
	Color col_back = {255,180,140,255};
	s->btn = button_create(SPRITE_STATION001, 0, s->name, s->pos.x, s->pos.y, size, size);
	button_set_click_callback(s->btn, button_callback, s);
	button_set_txt_antirot(s->btn, 1);
	button_set_backcolor(s->btn, col_back);
	button_set_animated(s->btn, 1, (i ? 18 : 15));
	button_set_enlargement(s->btn, 1.5);
	button_set_hotkeys(s->btn, digit2scancode[(i+1) % 10], 0);
	state_register_touchable(state_stations, s->btn);
	sprite *spr = button_get_sprite(s->btn);
	spr->antirot = 1;
	llist_add(sol->stations, s);
}

void solarsystem_remove_station(solarsystem *sy, station *s)
{
	if(llist_remove(sy->stations, s)) {
		if(state_remove_touchable(state_stations, s->btn)){
			SDL_Log("Could not remove station touchable");
		}
	}else{
		SDL_Log("Could not remove station");
	}
}

void solarsystem_register_touch(solarsystem *sol, STATE_ID id)
{
	/*llist_begin_loop(sol->stations);
	while (llist_hasnext(sol->stations)) {
		state_register_touchable(id,((station *)llist_next(sol->stations))->btn);
	}
	llist_end_loop(sol->stations);*/
}

void solarsystem_update(solarsystem *sol)
{
	llist_begin_loop(sol->stations);
	while (llist_hasnext(sol->stations)) {
		station *s = ((station *)llist_next(sol->stations));
		s->angle += 1/s->radius;
		s->pos = cpvadd(WE_P2C(s->radius,s->angle), sol->origo);
		touch_place(s->btn,s->pos.x, s->pos.y);
	}
	llist_end_loop(sol->stations);

}

void solarsystem_draw(solarsystem *sol)
{
	sun_render(RLAY_GAME_BACK, &sol->sun);
	llist_begin_loop(sol->stations);
	draw_color4b(50,50,50,50);
	while (llist_hasnext(sol->stations)) {
		cpVect p = ((station *)llist_next(sol->stations))->pos;
		float r = cpvlength(cpvsub(sol->origo,p));
		float w = 1 / current_view->zoom;
		draw_donut(RLAY_GAME_BACK, sol->origo, r - w, r + w);
	}
	llist_end_loop(sol->stations);
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

solarsystem * solarsystem_get_from_folder(LList systems, char *folder)
{
	solarsystem *solar = NULL;
	llist_begin_loop(systems);
	while (llist_hasnext(systems)) {
		solarsystem *s = llist_next(systems);
		if (strcmp(s->folder, folder) == 0) {
			solar = s;
			break;
		}
	}
	llist_end_loop(systems);
	return solar;
}

station * solarsystem_get_station(solarsystem *ss, char *path)
{
	station *found_st = NULL;
	llist_begin_loop(ss->stations);
	while (llist_hasnext(ss->stations)) {
		station *st = llist_next(ss->stations);
		SDL_Log("%s == %s", st->path, path);
		if (strcmp(st->path, path) == 0) {
			found_st = st;
			break;
		}
	}
	llist_end_loop(ss->stations);
	return found_st;
}


void solarsystem_destroy(solarsystem *sy)
{
	llist_set_remove_callback(sy->stations, (ll_rm_callback)station_free);
	llist_destroy(sy->stations);
	free(sy);
}
