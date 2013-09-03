#include "object_types.h"
#include "chipmunk.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"

#define OBJ_NAME turret
#include "we_defobj.h"

/* static prototypes */
static void velfunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

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

	turret->data.body = cpSpaceAddBody(space,
			cpBodyNew(100, cpMomentForBox(100.0f, TURRET_SIZE, TURRET_SIZE)));
	cpBodySetPos(turret->data.body, cpv(turret->data.p_start.x,currentlvl->height - TURRET_SIZE/2));

	turret->data.body->velocity_func = velfunc;

	/* make and connect new shape to body */
	shape_add_shapes(space, POLYSHAPE_TURRET, turret->data.body, TURRET_SIZE, 1, 0.7, turret, &this, LAYER_BUILDING, 2);

	cpBodySetUserData(turret->data.body, turret);

	hpbar_init(&turret->hp_bar,turret->param.max_hp,80,20,0,60,&(turret->data.body->p));
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
	if(turret->shooting && turret->timer > turret->rate && se_distance_to_player(turret->data.body->p.x) < turret->max_distance){
		turret->bullets += 1;

		cpVect shoot_vel = cpvforangle(turret->barrel_angle);
		cpVect shoot_pos = cpvadd(turret->data.body->p, cpvmult(shoot_vel,40));
		shoot_vel = cpvmult(shoot_vel, SHOOT_VEL);

		obj_param_bullet opb = {.friendly = 0, .damage = 10};
		instance_create(obj_id_bullet, &opb, shoot_pos.x, shoot_pos.y, shoot_vel.x, shoot_vel.y);

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
	draw_texture(texture, &(turret->data.body->p), &tex_map[0][0],TURRET_SIZE, TURRET_SIZE, dir*(180/M_PI));
	draw_texture(texture, &(turret->data.body->p), &tex_map[1][0],TURRET_SIZE, TURRET_SIZE, turret->barrel_angle*(180/M_PI));

	hpbar_draw(&turret->hp_bar);
}


static void velfunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpVect g = cpv(0,100);

	cpBodyUpdateVelocity(body, g, damping, dt);
}

static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	se_spawn_coins(turret);
	instance_remove(turret);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	cpBodyEachShape(turret->data.body,shape_from_space,NULL);

	cpSpaceRemoveBody(space, turret->data.body);
	cpBodyFree(turret->data.body);

	instance_super_free((instance *)turret);
}
