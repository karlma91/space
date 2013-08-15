/* standard c-libraries */
#include <stdio.h>
#include <math.h>

#include "object_types.h"
#define OBJ_NAME turret
#include "../../engine/components/object.h"

/* Chipmunk physics library */
#include "chipmunk.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/statesystem.h"
#include "../../engine/io/waffle_utils.h"

#include "../../engine/audio/sound.h"

/* Game state */
#include "../states/space.h"

/* Drawing */
#include "../../engine/graphics/draw.h"


#include "../../engine/graphics/particles.h"

#include "chipmunk.h"
#include "../spaceengine.h"

/* static prototypes */
static void velfunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

static const float tex_map[2][8] = {
		{0,1, 0.5,1, 0,0, 0.5,0},
		{0.5,1, 1,1, 0.5,0, 1,0}
};


static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	turret->data.components.hp_bar = &(turret->hp_bar);
	turret->data.components.score = &(turret->param.score);
	turret->data.components.body_count = 0;
	turret->data.alive = 1;
	turret->timer = 0;
	turret->rate = 0.060;
	turret->bullets = 0;
	turret->barrel_angle = 3*(M_PI/2);
	turret->max_distance = 800;

	cpFloat size = 100;
	turret->data.body = cpSpaceAddBody(space,
			cpBodyNew(500, cpMomentForBox(5000.0f, size, size)));
	cpBodySetPos(turret->data.body, cpv(turret->data.x,currentlvl->height - size/2));

	turret->data.body->velocity_func = velfunc;

	/* make and connect new shape to body */
	turret->shape = cpSpaceAddShape(space,cpBoxShapeNew(turret->data.body, size, size));

	//cpShapeSetGroup(fac->shape, 10);
	cpShapeSetFriction(turret->shape, 1);
	cpShapeSetLayers(turret->shape, LAYER_TANK_FACTORY);
	cpShapeSetCollisionType(turret->shape, obj_id_factory->ID);
	cpBodySetUserData(turret->data.body, turret);

	hpbar_init(&turret->hp_bar,turret->param.max_hp,80,16,-40,60,&(turret->data.body->p));
}


static void on_update(OBJ_TYPE *OBJ_NAME)
{
	/* get target: first player instance added */
	obj_player *player = ((obj_player*)instance_first(obj_id_player));

	cpVect pl = player->data.body->p;
	cpVect rc = turret->data.body->p;

	cpFloat best_angle = se_get_best_shoot_angle(rc,turret->data.body->v, pl, player->data.body->v, 3000);

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
		shoot_vel = cpvmult(shoot_vel, 1350);

		instance_create(obj_id_bullet, NULL, shoot_pos.x, shoot_pos.y, shoot_vel.x, shoot_vel.y);

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
	draw_texture(texture, &(turret->data.body->p), &tex_map[0][0],100, 100, dir*(180/M_PI));
	draw_texture(texture, &(turret->data.body->p), &tex_map[1][0],100, 100, turret->barrel_angle*(180/M_PI));

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
	cpBodyEachShape(turret->data.body,shape_from_space,NULL);

	cpSpaceRemoveBody(space, turret->data.body);
	cpBodyFree(turret->data.body);

	instance_super_free((instance *)turret);
}
