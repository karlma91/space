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

#define TURRET_SIZE 200
#define SHOOT_VEL 1500

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(turret, HPBAR, &turret->hp_bar);
	COMPONENT_SET(turret, COINS, &turret->param.coins);
	COMPONENT_SET(turret, MINIMAP, &turret->radar_image);
	turret->radar_image = cmp_new_minimap(10, COL_BLUE);

	turret->timer = 0;
	turret->rate = 0.060;
	turret->bullets = 0;
	turret->barrel_angle = 3*(M_PI/2);
	turret->max_distance = 800;

	turret->tower = cpSpaceAddBody(space, cpBodyNew(100, cpMomentForBox(100, TURRET_SIZE, TURRET_SIZE)));
	cpBodySetUserData(turret->tower, turret);
	cpBodySetPos(turret->tower, turret->data.p_start);
	se_tangent_body(turret->tower);
	se_velfunc(turret->tower, 1);
	shape_add_shapes(space, POLYSHAPE_TURRET, turret->tower, TURRET_SIZE, 1, 0.7, turret, &this, LAYER_BUILDING, 1);

	turret->data.body = cpSpaceAddBody(space, cpBodyNew(100, cpMomentForCircle(100.0f, 0, TURRET_SIZE,cpvzero)));
	cpBodySetUserData(turret->data.body, turret);
	cpBodySetPos(turret->data.body, turret->data.p_start);
	se_tangent_body(turret->data.body);
	se_velfunc(turret->data.body, 1);
	shape_add_shapes(space, POLYSHAPE_TURRET, turret->data.body, TURRET_SIZE, 1, 0.7, turret, &this, LAYER_BUILDING, 2);

	hpbar_init(&turret->hp_bar,turret->param.max_hp,80,20,0,60,&(turret->data.body->p));
	sprite_create(&turret->data.spr, SPRITE_TURRET, TURRET_SIZE, TURRET_SIZE, 0);
}


static void on_update(OBJ_TYPE *OBJ_NAME)
{
	/* get target: first player instance added */
	obj_player *player = ((obj_player*)instance_first(obj_id_player));
	if (player == NULL) {
		return;
	}

	cpVect pl = player->data.body->p;
	cpVect rc = turret->data.body->p;

	cpFloat best_angle = se_get_best_shoot_angle(rc,turret->data.body->v, pl, player->data.body->v, SHOOT_VEL);

	best_angle = best_angle - cpvtoangle(turret->data.body->rot);
	if(best_angle < 0){
		best_angle += 2*M_PI;
	}else if(best_angle>2*M_PI){
		best_angle -= 2*M_PI;
	}
	turret->barrel_angle = turn_toangle(turret->barrel_angle, best_angle, turret->param.rot_speed * dt);

	if(turret->timer > turret->param.shoot_interval){
		turret->shooting = 1;
		turret->timer = 0;
	}
	if(turret->shooting && turret->timer > turret->rate && se_arcdist2player(turret->data.body->p.x) < turret->max_distance){
		turret->bullets += 1;

		cpVect shoot_vel = cpvforangle(turret->barrel_angle);
		cpVect shoot_pos = cpvadd(turret->data.body->p, cpvmult(shoot_vel,40));
		shoot_vel = cpvmult(shoot_vel, SHOOT_VEL);

		obj_param_bullet opb = {.friendly = 0, .damage = 10};
		instance_create(obj_id_bullet, &opb, shoot_pos, shoot_vel);

		//TODO add the following data as params to bullet
		//turret->data.body->p
		//shoot_angle
		//cpVect shoot_angle = cpvforangle(turret->barrel_angle + cpBodyGetAngle(turret->data.body));
		//turret->data.body->v
		//ID_BULLET_ENEMY)

		if(turret->bullets > turret->param.burst_number){
			turret->bullets = 0;
			turret->shooting = 0;
		}
		turret->timer = 0;
	}else{
		turret->timer +=dt;
	}

}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	int texture = turret->param.tex_id;
	GLfloat dir = cpBodyGetAngle(turret->data.body);

	draw_color4f(1,1,1,1);
	sprite_set_index(&turret->data.spr, 0);
	sprite_render_body(&turret->data.spr, turret->tower);
	sprite_set_index(&turret->data.spr, 1);
	sprite_render_body(&turret->data.spr, turret->data.body);

	hpbar_draw(&turret->hp_bar);
}

static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	sound_play(SND_FACTORY_EXPLODE);
	se_spawn_coins(turret);
	instance_remove(turret);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(space, &turret->data.body);
	we_body_remove(space, &turret->tower);
}
