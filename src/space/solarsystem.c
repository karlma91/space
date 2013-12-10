#include "solarsystem.h"
#include "game.h"


solarsystem *solarsystem_create(view *cam, int solsys_index, float star_size, SPRITE_ID star_spr, Color star_base, Color star_glow, Color star_add1, Color star_add2)
{
	solarsystem *solsys = (solarsystem *)calloc(1, sizeof *solsys);

	solsys->cam = cam;
	solsys->index = solsys_index;

	cpFloat radius = solsys_index*solsys_index * 6500;
	cpFloat angle = WE_2PI * solsys_index / 17.371;

	solsys->origo = WE_P2C(radius,angle);

	solsys->station_count = 0;
	solsys->sun.size = star_size;
	solsys->sun.base = star_base;
	solsys->sun.glow = star_glow;
	solsys->sun.add1 = star_add1;
	solsys->sun.add2 = star_add2;
	solsys->sun.pos = solsys->origo;
	solsys->sun.spr_id = star_spr;

	return solsys;
}

void solarsystem_add_station(SPRITE_ID spr_id)
{
//TODO implement stations
}

void solarsystem_update(solarsystem *sun)
{

}

void solarsystem_draw(solarsystem *sun)
{
	sun_render(RLAY_GAME_BACK, sun);
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

