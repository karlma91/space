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

/* static prototypes */
static void init(object_group_tank *);
static void update(object_group_tank *);
static void render(object_group_tank *);
static void destroy(object_group_tank *);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);
static cpBody *addChassis(cpSpace *space, object_group_tank *tank, cpVect pos, cpVect boxOffset);
static cpBody *addWheel(cpSpace *space, cpVect pos, cpVect boxOffset);

object_group_preset type_tank= {
	ID_TANK,
	init,
	update,
	render,
	destroy
};

static const texture_map tex_map[2] = {
		{0,0,1,0.5}, {0,0.5,0.5,1}
};

object_group_tank *object_create_tank(float xpos,object_group_tankfactory *factory, object_param_tank *param)
{
	//TODO use pointer value as group id
	object_group_tank *tank = malloc(sizeof(*tank));
	tank->data.preset = &type_tank;
	tank->data.alive = 1;
	tank->param = param;

	tank->timer = 0;
	tank->factory = factory;

	cpFloat height = 30;
	if (factory){
		tank->factory_id = factory->data.instance_id;
		height = factory->data.body->p.y;
	}

	tank->rot_speed = 0.01;
	tank->angle = 0;


	// Make a car with some nice soft suspension
	cpVect boxOffset = cpv(0, 0);
	cpVect posA = cpv(xpos-20, 120);
	cpVect posB = cpv(xpos+20, 120);

	tank->data.body = addChassis(space, tank, cpv(xpos, height+10), boxOffset);

	tank->debug_left_dist = -1;
	tank->debug_right_dist = -1;

	cpShapeSetCollisionType(tank->shape, ID_TANK);
	cpSpaceAddCollisionHandler(space, ID_TANK, ID_BULLET_PLAYER, collision_player_bullet, NULL, NULL, NULL, NULL);

	tank->wheel1 = addWheel(space, posA, boxOffset);
	tank->wheel2 = addWheel(space, posB, boxOffset);

	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel1 , cpv(-30, -10), cpv(-30, -40), cpvzero));
	cpSpaceAddConstraint(space, cpGrooveJointNew(tank->data.body, tank->wheel2, cpv( 30, -10), cpv( 30, -40), cpvzero));

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
	cpBodySetTorque(tank->wheel2,velocity);
}

static void update(object_group_tank *tank)
{
	tank->timer +=dt;

	/* gets the player from the list */
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

	//TODO fix best_angle
	cpFloat player_angle = se_get_best_shoot_angle(tank->data.body, player->data.body, 3000);

	//TODO bruke generell vinkel metode som u player.c!

	/*TODO: stop shaking when at correct angle */
		/* som i matlab */
		tank->angle += ((player_angle > tank->angle)*2 - 1) * tank->rot_speed;

	if(tank->timer > 1 + ((3.0f*rand())/RAND_MAX)){
		cpVect t = cpvforangle(tank->angle );
		bullet_init(tank->data.body->p,t,tank->data.body->v,ID_BULLET_ENEMY);
		tank->timer = 0;
	}

	cpFloat tx = tank->data.body->p.x;
	cpFloat px = player->data.body->p.x;

	cpFloat ptx = (px-tx); //direct way
	cpFloat pltx = (tx - currentlvl->left + (currentlvl->right - px));
	cpFloat prtx = (currentlvl->right - tx + (px - currentlvl->left));
	if(fabs(ptx) < prtx && fabs(ptx) < pltx){
		ptx = ptx>0? 1:-1;
	}else if(pltx < prtx){
		ptx = -1;
	}else {
		ptx = 1;
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
				set_wheel_velocity(tank, left_dist > 400 ? 50 : 20);
			else if (right_clear)
				set_wheel_velocity(tank, right_dist > 400 ? -50 : -20);
			else
				set_wheel_velocity(tank, 0);
		} else if (ptx > 0) {
			if (right_clear)
				set_wheel_velocity(tank, right_dist > 400 ? -50 : -20);
			else if (left_clear)
				set_wheel_velocity(tank, left_dist > 400 ? 50 : 20);
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

	tank->shape = se_add_box_shape(body,width,height,0.7,0.0);

	//cpShapeSetGroup(tempShape, 1); // use a group to keep the car parts from colliding

	cpShapeSetLayers(tank->shape, LAYER_TANK);

	return body;
}

static void render(object_group_tank *tank)
{
	if (tank->param->max_hp < 100)
		glColor4f(1,1,1,0.6);
	else
		glColor4f(1,1,0,0.6);

	glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	GLfloat dir = cpBodyGetAngle(tank->data.body)*(180/M_PI);
	GLfloat rot = cpBodyGetAngle(tank->wheel1)*(180/M_PI);

	cpVect r = cpvadd(tank->data.body->p, cpvmult(cpvforangle(tank->angle),80));
	draw_line(tank->data.body->p.x,tank->data.body->p.y,r.x,r.y, 30);

	hpbar_draw(&tank->hp_bar);

	int texture = tank->param->tex_id;

	draw_texture(texture, &(tank->data.body->p), &tex_map[0],200, 100, dir);
	draw_texture(texture, &tank->wheel1->p, &tex_map[1],100, 100, rot);
	draw_texture(texture, &tank->wheel2->p, &tex_map[1],100, 100, rot);

	glPopAttrib();
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	object_group_tank *tank = ((object_group_tank *)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->alive = 0;

	se_add_explotion_at_contact_point(arb);

	//TODO create a function for damaging other objects
	tank->hp_bar.value -= bt->damage;

	if (tank->hp_bar.value <= 0) {
		if (tank->data.alive) {
			particles_get_emitter_at(EMITTER_EXPLOSION, b->body->p);
			se_add_score_and_popup(b->body->p, tank->param->score);
		}
		//cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
		tank->data.alive = 0;
	}

	return 0;
}


static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}
static void constrain_from_space(cpBody *body, cpConstraint *constraint, void *data)
{
    cpSpaceRemoveConstraint(space, constraint);
    cpConstraintFree(constraint);
}

static void destroy(object_group_tank *tank)
{
	cpBodyEachShape(tank->data.body,shape_from_space,NULL);
	cpBodyEachConstraint(tank->data.body,constrain_from_space,NULL);

	cpSpaceRemoveBody(space, tank->data.body);
	cpBodyFree(tank->data.body);

	cpBodyEachShape(tank->wheel1,shape_from_space,NULL);
	cpBodyEachShape(tank->wheel2,shape_from_space,NULL);
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
