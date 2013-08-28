/* standard c-libraries */
#include <stdio.h>
#include <math.h>

#include "object_types.h"
#define OBJ_NAME tank
#include "../../engine/components/object.h"

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
static cpBody *addChassis(cpSpace *space, obj_tank *tank, cpVect pos, cpVect boxOffset, cpGroup group);
static cpBody *addWheel(cpSpace *space, cpVect pos, cpVect boxOffset, cpGroup group);

#define MASS 5.0f
#define MASS_WHEEL 10.0f
#define SHOOT_VEL 1500

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(tank, HPBAR, &tank->hp_bar);
	COMPONENT_SET(tank, COINS, &tank->param.coins);
	COMPONENT_SET(tank, MINIMAP, &tank->radar_image);
	tank->radar_image = cmp_new_minimap(10, COL_RED);

	sprite_create(&(tank->wheel_sprite), SPRITE_TANK_WHEEL, 120, 120, 0);
	sprite_create(&(tank->data.spr), SPRITE_TANK_BODY, 200, 100, 0);
	sprite_create(&(tank->turret_sprite), SPRITE_TANK_TURRET, 150, 150, 0);

	tank->max_distance = 800;

	cpFloat start_height = 80; // default start height (if spawned without factory)

	if (tank->factory){
		tank->factory_id = tank->factory->data.instance_id;
		start_height = tank->factory->data.body->p.y - 100;
	}

	cpVect boxOffset = cpvzero;
	tank->data.p_start.y = start_height + 10;
	tank->rot_speed = M_PI/2;
	tank->data.body = addChassis(space, tank,tank->data.p_start, boxOffset, tank);

	// Make a car with some nice soft suspension
	float wheel_offset = 40;
	cpVect posA = cpv(tank->data.body->p.x - wheel_offset, start_height - 25);
	cpVect posB = cpv(tank->data.body->p.x + wheel_offset, start_height - 25);


	tank->debug_left_dist = -1;
	tank->debug_right_dist = -1;

	cpShapeSetCollisionType(tank->shape, &this);

	tank->wheel1 = addWheel(space, posA, boxOffset, tank);
	tank->wheel2 = addWheel(space, posB, boxOffset, tank);

	tank->data.components[CMP_BODIES] = tank->wheel1;
	tank->data.components[CMP_BODIES+1] = tank->wheel2;

	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel1 , cpv(-30, -10), cpv(-wheel_offset, -40), cpvzero));
	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel2, cpv( 30, -10), cpv( wheel_offset, -40), cpvzero));

	cpSpaceAddConstraint(space, cpDampedSpringNew(tank->data.body, tank->wheel1 , cpv(-30, 0), cpvzero, 50.0f, 600.0f, 0.5f));
	cpSpaceAddConstraint(space, cpDampedSpringNew(tank->data.body, tank->wheel2, cpv( 30, 0), cpvzero, 50.0f, 600.0f, 0.5f));

	cpBodySetUserData(tank->data.body, tank);

	hpbar_init(&tank->hp_bar,tank->param.max_hp,80,16,-40,60,&(tank->data.body->p));
}

static void set_wheel_velocity(obj_tank *tank, float velocity)
{
	cpBodySetAngVel(tank->wheel1,velocity);
	cpBodySetAngVel(tank->wheel2,velocity);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{

#if !ARCADE_MODE
	if (keys[SDL_SCANCODE_F2]) {
		tank->data.body->p.x = 0;
		tank->data.body->p.y = 200;
	}
#endif

	tank->timer +=dt;
	/* gets the player from the list */
	obj_player *player = ((obj_player*)instance_first(obj_id_player));

	cpVect pl = player->data.body->p;
	cpVect rc = tank->data.body->p;
	float ptx = se_distance_to_player(tank->data.body->p.x);
	if(ptx > 0 && ptx > currentlvl->right - rc.x){
		pl.x += currentlvl->width;
	}else if(ptx < 0 && ptx > rc.x - currentlvl->left ){
		pl.x -= currentlvl->width;
	}

	cpFloat best_angle = se_get_best_shoot_angle(rc,tank->data.body->v, pl, player->data.body->v, SHOOT_VEL);

	best_angle = best_angle - cpvtoangle(tank->data.body->rot);
	if(best_angle < 0){
		best_angle += 2*M_PI;
	}else if(best_angle>2*M_PI){
		best_angle -= 2*M_PI;
	}
	tank->barrel_angle=turn_toangle(tank->barrel_angle, best_angle, tank->rot_speed * dt);


	if(tank->barrel_angle > M_PI && tank->barrel_angle < 3*(M_PI/2)){
		tank->barrel_angle = M_PI;
	}else if(tank->barrel_angle<0 || tank->barrel_angle > 3*(M_PI/2)){
		tank->barrel_angle = 0;
	}

	if(tank->timer > 1 + ((3.0f*rand())/RAND_MAX) && se_distance_to_player(tank->data.body->p.x)<tank->max_distance){
		//TODO hent ut lik kode for skyting og lag en metode av det
		cpVect shoot_vel = cpvforangle(tank->barrel_angle + cpBodyGetAngle(tank->data.body));
		cpVect shoot_pos = cpvadd(tank->data.body->p, cpvmult(shoot_vel,55));

		shoot_vel = cpvmult(shoot_vel,SHOOT_VEL);

		obj_param_bullet opb = {.friendly = 0, .damage = 10};
		instance_create(obj_id_bullet, &opb, shoot_pos.x, shoot_pos.y, shoot_vel.x, shoot_vel.y);
		//object_create_bullet(tank->data.body->p,shoot_angle ,tank->data.body->v,obj_id_bullet);
		sound_play(SND_LASER_2);
		tank->timer = 0;
	}


	instance *left, *right;
	cpFloat left_dist, right_dist;
	instance_nearest_x_two((instance *)tank, obj_id_tank, &left, &right, &left_dist, &right_dist);

	int left_clear = (left_dist > 250);
	int right_clear = (right_dist > 250);

	tank->debug_left_dist = left ? left_dist : -1;
	tank->debug_right_dist = right ? right_dist : -1;

	//TMP DEBUG OVERSTYRING AV TANK
	if (keys[SDL_SCANCODE_LCTRL]) {
		if (keys[SDL_SCANCODE_K])
			set_wheel_velocity(tank, 500);
		else if (keys[SDL_SCANCODE_L])
			set_wheel_velocity(tank, -500);
		else
			set_wheel_velocity(tank, 0);
	} else {
		if (ptx < 0) {
			if (left_clear)
				set_wheel_velocity(tank, left_dist > 400 ? 20 : 5);
			else if (right_clear)
				set_wheel_velocity(tank, right_dist > 400 ? -20 : -5);
			else
				set_wheel_velocity(tank, 0);
		} else if (ptx > 0) {
			if (right_clear)
				set_wheel_velocity(tank, right_dist > 400 ? -20 : -5);
			else if (left_clear)
				set_wheel_velocity(tank, left_dist > 400 ? 20 : 5);
			else
				set_wheel_velocity(tank, 0);
		} else {
			set_wheel_velocity(tank, 0);
		}
	}
}

/*
 * make a wheel
 */
static cpBody * addWheel(cpSpace *space, cpVect pos, cpVect boxOffset, cpGroup group) {
	cpFloat radius = 15.0f;
	cpBody *body = cpSpaceAddBody(space,
			cpBodyNew(MASS_WHEEL, cpMomentForCircle(MASS_WHEEL, 0.0f, radius, cpvzero)));
	cpBodySetPos(body, cpvadd(pos, boxOffset));
	cpBodySetAngVelLimit(body, 200);

	cpShape *shape = se_add_circle_shape(body, radius, 0.8, 0.7);
	cpShapeSetGroup(shape, &this);
	cpShapeSetLayers(shape, LAYER_BULLET_ENEMY);

	return body;
}

/**
 * make a chassies
 */
static cpBody *addChassis(cpSpace *space, obj_tank *tank, cpVect pos, cpVect boxOffset, cpGroup group)
{
	cpFloat width = 80;
	cpFloat height = 30;

	cpBody *body = cpSpaceAddBody(space, cpBodyNew(MASS, cpMomentForBox(MASS, width, height)));
	cpBodySetPos(body, cpvadd(pos, boxOffset));

	tank->shape = se_add_box_shape(body,width,height,0.6,0.0);
	cpShapeSetGroup(tank->shape, group);
	cpShapeSetLayers(tank->shape, LAYER_ENEMY);

	return body;
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{

	GLfloat dir = cpBodyGetAngle(tank->data.body);
	GLfloat rot = cpBodyGetAngle(tank->wheel1)*(180/M_PI);
	GLfloat barrel_angle = (tank->barrel_angle + dir) * (180/M_PI);

	hpbar_draw(&tank->hp_bar);

	draw_color4f(1,1,1,1);

	cpVect pos_w1 = tank->wheel1->p;
	sprite_render(&(tank->wheel_sprite), &pos_w1, rot);

	cpVect pos_w2 = tank->wheel2->p;
	sprite_render(&(tank->wheel_sprite), &pos_w2, rot);

	if (tank->param.max_hp >= 100) {//TODO add color into param
		draw_color4f(1,0.2,0,1);
	}

	cpVect pos = tank->data.body->p;

	sprite_render(&(tank->data.spr), &pos, dir*180/M_PI);
	sprite_render(&(tank->turret_sprite), &pos, barrel_angle);
}


static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	particles_get_emitter_at(EMITTER_FRAGMENTS, tank->data.body->p);
	se_spawn_coins((instance *)tank);

	cpBodyEachShape(tank->data.body,se_shape_from_space,NULL);
	cpBodyEachConstraint(tank->data.body,se_constrain_from_space,NULL);

	cpSpaceRemoveBody(space, tank->data.body);
	cpBodyFree(tank->data.body);

	cpBodyEachShape(tank->wheel1,se_shape_from_space,NULL);
	cpBodyEachShape(tank->wheel2,se_shape_from_space,NULL);
	cpSpaceRemoveBody(space, tank->wheel1);
	cpSpaceRemoveBody(space, tank->wheel2);
	cpBodyFree(tank->wheel1);
	cpBodyFree(tank->wheel2);

	if(tank->factory != NULL){
		tank->factory->cur--;
	}

	instance_super_free((instance *)tank);
}
