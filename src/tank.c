/* header */
#include "tank.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Chipmunk physics library */
#include "chipmunk.h"

/* Game state */
#include "space.h"

/* Drawing */
#include "draw.h"
#include "particles.h"
#include "font.h"

#include "objects.h"

/* Game components */
#include "player.h"
#include "bullet.h"
#include "spaceengine.h"
#include "collisioncallbacks.h"

/* static prototypes */
static void init(object_group_tank *);
static void update(object_group_tank *);
static void render(object_group_tank *);
static void destroy(object_group_tank *);
static cpBody *addChassis(cpSpace *space, object_group_tank *tank, cpVect pos, cpVect boxOffset);
static cpBody *addWheel(cpSpace *space, cpVect pos, cpVect boxOffset);

object_group_preset type_tank= {
	ID_TANK,
	init,
	update,
	render,
	destroy
};

static const texture_map tex_map[3] = {
		{0,0,1,0.5}, {0,0.5,0.5,1}, {0.5,0.5,1,1}
};

object_group_tank *object_create_tank(float xpos, object_group_factory *factory, object_param_tank *param)
{
	//TODO use pointer value as group id
	object_group_tank *tank = malloc(sizeof(*tank));
	tank->data.preset = &type_tank;
	tank->data.components.hp_bar = &(tank->hp_bar);
	tank->data.components.score = &(param->score);
	tank->data.components.body_count = 2;
	tank->data.alive = 1;
	tank->param = param;

	tank->max_distance = 800;

	tank->timer = 0;
	tank->factory = factory;
	tank->data.destroyed = 0;

	cpFloat start_height = 30;
	if (factory){
		tank->factory_id = factory->data.instance_id;
		start_height = factory->data.body->p.y;
	}

	tank->rot_speed = M_PI/2;
	tank->barrel_angle = 0;


	float wheel_offset = 40;

	// Make a car with some nice soft suspension
	cpVect boxOffset = cpv(0, 0);
	cpVect posA = cpv(xpos-wheel_offset, start_height-25);
	cpVect posB = cpv(xpos+wheel_offset, start_height-25);

	tank->data.body = addChassis(space, tank, cpv(xpos, start_height+10), boxOffset);

	tank->debug_left_dist = -1;
	tank->debug_right_dist = -1;

	cpShapeSetCollisionType(tank->shape, ID_TANK);

	tank->wheel1 = addWheel(space, posA, boxOffset);
	tank->wheel2 = addWheel(space, posB, boxOffset);

	tank->data.components.bodies[0] = tank->wheel1;
	tank->data.components.bodies[1] = tank->wheel2;

	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel1 , cpv(-30, -10), cpv(-wheel_offset, -40), cpvzero));
	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel2, cpv( 30, -10), cpv( wheel_offset, -40), cpvzero));

	cpSpaceAddConstraint(space, cpDampedSpringNew(tank->data.body, tank->wheel1 , cpv(-30, 0), cpvzero, 50.0f, 60.0f, 0.5f));
	cpSpaceAddConstraint(space, cpDampedSpringNew(tank->data.body, tank->wheel2, cpv( 30, 0), cpvzero, 50.0f, 60.0f, 0.5f));

	cpBodySetUserData(tank->data.body, tank);
	objects_add((object_data *)tank);

	hpbar_init(&tank->hp_bar,param->max_hp,80,16,-40,60,&(tank->data.body->p));

	return tank;
}


static void init(object_group_tank *tank)
{
}

static void set_wheel_velocity(object_group_tank *tank, float velocity)
{
	cpBodySetAngVel(tank->wheel1,velocity);
	cpBodySetAngVel(tank->wheel2,velocity);
}

static void update(object_group_tank *tank)
{
	tank->timer +=dt;
	/* gets the player from the list */
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

	cpVect pl = player->data.body->p;
	cpVect rc = tank->data.body->p;

	cpFloat ptx = (pl.x-rc.x); //direct way
	cpFloat pltx = (rc.x - currentlvl->left + (currentlvl->right - pl.x));
	cpFloat prtx = (currentlvl->right - rc.x + (pl.x - currentlvl->left));
	if(fabs(ptx) < prtx && fabs(ptx) < pltx){
		ptx = ptx>0? 1:-1;
	}else if(pltx < prtx){
		pl.x -= currentlvl->right - currentlvl->left;
		ptx = -1;
	}else {
		pl.x += currentlvl->right - currentlvl->left;
		ptx = 1;
	}

	cpFloat best_angle = se_get_best_shoot_angle(rc,tank->data.body->v, pl, player->data.body->v, 3000);

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
		cpVect shoot_angle = cpvforangle(tank->barrel_angle + cpBodyGetAngle(tank->data.body));
		object_create_bullet(tank->data.body->p,shoot_angle ,tank->data.body->v,ID_BULLET_ENEMY);
		tank->timer = 0;
	}


	object_data *left, *right;
	cpFloat left_dist, right_dist;
	objects_nearest_x_two((object_data *)tank, ID_TANK, &left, &right, &left_dist, &right_dist);

	int left_clear = (left_dist > 250);
	int right_clear = (right_dist > 250);

	tank->debug_left_dist = left ? left_dist : -1;
	tank->debug_right_dist = right ? right_dist : -1;

	//TMP DEBUG OVERSTYRING AV TANK
	if (keys[SDLK_LCTRL]) {
		if (keys[SDLK_k])
			set_wheel_velocity(tank, 500);
		else if (keys[SDLK_l])
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
static cpBody * addWheel(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat radius = 15.0f;
	cpFloat mass = 1.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(body, cpvadd(pos, boxOffset));
	cpBodySetAngVelLimit(body,200);

	cpShape *shape = se_add_circle_shape(body,radius,0.7,0.0);

	cpShapeSetGroup(shape, 1); // use a group to keep the car parts from colliding

	cpSpaceAddCollisionHandler(space, ID_TANK, ID_ABSTRACT_WHEEL, NULL, NULL, NULL, NULL, NULL);

	cpShapeSetLayers(shape,LAYER_WHEEL);

	return body;
}

/**
 * make a chassies
 */
static cpBody *addChassis(cpSpace *space, object_group_tank *tank, cpVect pos, cpVect boxOffset)
{
	cpFloat mass = 2.0f;
	cpFloat width = 80;
	cpFloat height = 30;

	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPos(body, cpvadd(pos, boxOffset));

	tank->shape = se_add_box_shape(body,width,height,0.6,0.0);

	//cpShapeSetGroup(tempShape, 1); // use a group to keep the car parts from colliding

	cpShapeSetLayers(tank->shape, LAYER_TANK);

	return body;
}

static void render(object_group_tank *tank)
{

	glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	GLfloat dir = cpBodyGetAngle(tank->data.body);
	GLfloat rot = cpBodyGetAngle(tank->wheel1)*(180/M_PI);
	GLfloat barrel_angle = (tank->barrel_angle + dir) * (180/M_PI);

	hpbar_draw(&tank->hp_bar);

	int texture = tank->param->tex_id;

		glColor4f(1,1,1,1);
		draw_texture(texture, &tank->wheel1->p, &tex_map[1],100, 100, rot);
		draw_texture(texture, &tank->wheel2->p, &tex_map[1],100, 100, rot);
	if (tank->param->max_hp < 100) {//TODO add color into param
		draw_texture(texture, &(tank->data.body->p), &tex_map[0],200, 100, dir*(180/M_PI));
		draw_texture(texture, &(tank->data.body->p), &tex_map[2],150, 150, barrel_angle);
	} else {
		glColor4f(1,0.2,0,1);
		draw_texture(texture, &(tank->data.body->p), &tex_map[0],200, 100, dir*(180/M_PI));
		draw_texture(texture, &(tank->data.body->p), &tex_map[2],150, 150, barrel_angle);
	}
	glPopAttrib();
}


static void destroy(object_group_tank *tank)
{
	particles_get_emitter_at(EMITTER_FRAGMENTS, tank->data.body->p);

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

	objects_remove((object_data *)tank);
	free(tank);
}
