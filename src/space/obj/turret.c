#include "object_types.h"
#include "chipmunk.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"

#define OBJ_NAME turret
#include "we_defobj.h"

static const float tex_map[2][8] = {
		{0,1, 0.5,1, 0,0, 0.5,0},
		{0.5,1, 1,1, 0.5,0, 1,0}
};

static void init(OBJ_TYPE *OBJ_NAME)
{
	float turret_size = turret->param.turret_size;
	cpVect p_start = turret->data.p_start;
	p_start = we_cart2pol(p_start);
	p_start.x = currentlvl->inner_radius + turret_size/2;
	p_start = we_pol2cart(p_start);

	cpBodySetPos(turret->tower, p_start);
	se_tangent_body(turret->tower);
	cpBodySetPos(turret->data.body, p_start);
	se_tangent_body(turret->data.body);

	cpSpaceReindexShapesForBody(current_space, turret->tower);
	cpSpaceReindexShapesForBody(current_space, turret->data.body);
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	float turret_size = turret->param.turret_size;
	COMPONENT_SET(turret, HPBAR, &turret->hp_bar);
	COMPONENT_SET(turret, COINS, &turret->param.coins);
	COMPONENT_SET(turret, MINIMAP, &turret->radar_image);
	turret->radar_image = cmp_new_minimap(10, COL_BLUE);

	turret->timer = 0;
	turret->rate = 0.060;
	turret->bullets = 0;
	turret->max_distance = 600;

	cpVect p_start = turret->data.p_start;
	p_start = we_cart2pol(p_start);
	p_start.x = currentlvl->inner_radius + turret_size/2;
	p_start = we_pol2cart(p_start);

	turret->tower = cpSpaceAddBody(current_space, cpBodyNew(100, cpMomentForBox(100, turret_size, turret_size)));
	cpBodySetUserData(turret->tower, turret);
	cpBodySetPos(turret->tower, p_start);
	se_tangent_body(turret->tower);
	se_velfunc(turret->tower, -1);
	shape_add_shapes(current_space, POLYSHAPE_TURRET, turret->tower, turret_size, 100, cpvzero, 1, 0.7, turret, NULL, LAYER_BUILDING, 2);

	float mass = turret->param.mass;
	turret->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0, turret_size,cpvzero)));
	cpBodySetUserData(turret->data.body, turret);
	cpBodySetPos(turret->data.body, p_start);
	se_tangent_body(turret->data.body);
	se_velfunc(turret->data.body, 1);
	shape_add_shapes(current_space, POLYSHAPE_TURRET, turret->data.body, turret_size, mass, cpvzero, 1, 0.7, turret, &this, LAYER_ENEMY, 1);

	cpSpaceAddConstraint(current_space, cpPivotJointNew(turret->data.body, turret->tower, p_start));

	hpbar_init(&turret->hp_bar,turret->param.max_hp,80,20,0,60,&(turret->data.body->p));
	sprite_create(&turret->data.spr, SPRITE_TURRETBODY001, turret_size, turret_size, 0);
	sprite_create(&turret->spr_gun, SPRITE_TURRETGUN001, turret_size, turret_size, 0);

	turret->bullet_param = level_get_param(&currentlvl->params, turret->param.bullet_type->NAME, turret->param.bullet_param);

	init(turret);
}


static void on_update(OBJ_TYPE *OBJ_NAME)
{
	/* get target: first player instance added */
	obj_player *player = ((obj_player*) instance_nearest(turret->data.body->p, obj_id_player));
	if (player == NULL) {
		return;
	}

	cpFloat best_angle = se_get_best_shoot_angle(turret->data.body, player->data.body, turret->param.shoot_vel);

	turret->data.body->a = turn_toangle(turret->data.body->a, best_angle,  WE_2PI*dt*turret->param.rot_speed);
	cpVect turret_angle = cpvforangle(turret->data.body->a - turret->tower->a);
	turret->data.body->a = cpvdot(turret_angle, cpv(0,1)) > WE_PI_6/2 ? (turret->tower->a
					+ (cpvdot(turret_angle, cpv(1,0)) > 0 ? WE_PI_6/2 : WE_PI-WE_PI_6/2))  : turret->data.body->a;
	cpBodySetAngle(turret->data.body, turret->data.body->a);

	//TODO generalisere maskingevær skyting
	if (turret->timer > turret->param.shoot_interval) {
		turret->shooting = 1;
		turret->timer = 0;
	}
	if (turret->shooting && turret->timer > turret->rate
			&& cpvlengthsq(se_dist_a2b((instance*)turret, (instance*)player)) <  turret->max_distance * turret->max_distance) {
		turret->bullets += 1;

		cpVect shoot_vel = cpvmult(turret->data.body->rot, turret->param.shoot_vel);
		cpVect shoot_pos = cpvadd(turret->data.body->p, cpvmult(turret->data.body->rot, 40));

		instance_create(obj_id_bullet, turret->bullet_param, shoot_pos, shoot_vel);

		if (turret->bullets > turret->param.burst_number) {
			turret->bullets = 0;
			turret->shooting = 0;
		}
		turret->timer = 0;
	} else {
		turret->timer += dt;
	}

}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
	if (turret->data.time_destroyed > 2) {
		instance_remove((instance *)turret);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float alpha = 1;
	if (turret->data.time_destroyed > 0) {
		alpha = maxf(0, 1 - turret->data.time_destroyed / 2);
		alpha = 1- (1-alpha)*(1-alpha);
	}

	draw_color_rgbmulta4f(1,1,1,alpha);
	sprite_render_body(RLAY_GAME_MID, &turret->data.spr, turret->tower);
	sprite_render_body(RLAY_GAME_MID, &turret->spr_gun, turret->data.body);

	hpbar_draw(RLAY_GAME_FRONT, &turret->hp_bar, cpvtoangle(turret->data.body->p));
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	explosion_create(turret->data.body->p, EM_EXPLOSION, NULL, SND_BUILDING_EXPLODE, 1200, 160, 0.3,10);
	se_spawn_coins((instance *)turret);
	we_body_remove_constraints(current_space, turret->data.body);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &turret->data.body);
	we_body_remove(current_space, &turret->tower);
}
