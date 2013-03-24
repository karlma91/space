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

#include "objects.h"

/* Game components */
#include "player.h"
#include "bullet.h"

/* static prototypes */
static void init(object_group_tank *);
static void update(object_group_tank *);
static void render(object_group_tank *);
static void destroy(object_group_tank *);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);
static cpShape *tempShape; //todo remove this?
static cpBody *addChassis(cpSpace *space, cpVect pos, cpVect boxOffset);
static cpBody *addWheel(cpSpace *space, cpVect pos, cpVect boxOffset);

/* helper */
//static float get_angle(object *obj, object *obj2);
static float get_best_angle(object_data *obj, object_data *obj2);

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
	object_group_tank *tank = malloc(sizeof(*tank));
	tank->data.preset = &type_tank;
	tank->data.alive = 1;
	tank->param = param;

	tank->timer = 0;
	tank->factory = factory;

	if (factory)
		tank->factory_id = factory->data.instance_id;

	tank->rot_speed = 0.01;
	tank->angle = 0;

//	cpFloat width = 50;
	cpFloat height = 30;

	/* make and add new body */
	// Make a car with some nice soft suspension
	cpVect boxOffset;
	cpVect posA = cpv(xpos-20, 120);
	cpVect posB = cpv(xpos+20, 120);
	boxOffset = cpv(0, 0);

	tank->data.body = addChassis(space, cpv(xpos, height+10),boxOffset);

	tank->shape = tempShape;
	tempShape = NULL;
	//((object *) tank)->body = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(20.0f, width, height)));
	//cpBodySetPos(((object *) tank)->body, cpv(xpos,height+10));
	//cpBodySetVelLimit(((object *) tank)->body,180);
	/* make and connect new shape to body */
	//tank->shape = cpSpaceAddShape(space, cpBoxShapeNew(((object *) tank)->body, width, height));
	//cpShapeSetFriction(tank->shape, 0.01);
	//cpShapeSetGroup(tank->shape, 10);
	//cpShapeSetLayers(tank->shape,LAYER_TANK);
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

static void update(object_group_tank *tank)
{
	tank->timer +=dt;

	/* gets the player from the list */
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

	//TODO fix best_angle
	float player_angle = get_best_angle((object_data *)tank, (object_data *)player);

	//TODO bruke generell vinkel metode som u player.c!
	//TODO implement tank collision avoidance

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

	if(ptx<0){
		cpBodySetTorque(tank->wheel1,20000);
		cpBodySetTorque(tank->wheel2,20000);
	}else{
		cpBodySetTorque(tank->wheel1,-20000);
		cpBodySetTorque(tank->wheel2,-20000);
	}

}

/**
 * returns the best angle to shoot at a moving object obj2 from obj1
 */
static float get_best_angle(object_data *obj, object_data *obj2)
{
	cpVect a = cpvsub(obj->body->p, obj2->body->p);

	cpFloat c = cpvlength(obj2->body->v);
	cpFloat b = 1500;
	cpFloat G = acos(cpvdot(a,obj2->body->v)/(cpvlength(obj2->body->v)*cpvlength(a)));
	float angle = asin((c*sin(G))/b);

	cpFloat bc = cpvtoangle(a);

	if(obj2->body->v.x < 0){
		angle  = -angle;
	}
	angle  = M_PI + (bc  - angle );

	return angle;
}

//TODO: misplaced method: remove or move this method to objects?
/**
 * returns the angle from a object to another
 */
/*static float get_angle(object *obj, object *obj2)
{
	cpVect a = cpvsub(obj->body->p, obj2->body->p);
	cpFloat bc = cpvtoangle(a);
	return bc;
}
*/

/*
 * make a wheel
 */
static cpBody * addWheel(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat radius = 15.0f;
	cpFloat mass = 1.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(body, cpvadd(pos, boxOffset));
	cpBodySetAngVelLimit(body,20);

	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
	cpShapeSetGroup(shape, 1); // use a group to keep the car parts from colliding
	cpShapeSetLayers(shape,LAYER_TANK);

	return body;
}

/**
 * make a chassies
 */
static cpBody *
addChassis(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat mass = 2.0f;
	cpFloat width = 80;
	cpFloat height = 30;

	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPos(body, cpvadd(pos, boxOffset));

	tempShape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
	cpShapeSetElasticity(tempShape, 0.0f);
	cpShapeSetFriction(tempShape, 0.7f);
	cpShapeSetGroup(tempShape, 1); // use a group to keep the car parts from colliding
	cpShapeSetLayers(tempShape,LAYER_TANK);

	return body;
}

static void render(object_group_tank *tank)
{
	if (tank->param->max_hp < 100)
		glColor3f(1,1,1);
	else
		glColor3f(1.0,0.4,0.4);

	//draw_boxshape(temp->shape,RGBAColor(0.8,0.3,0.1,1),RGBAColor(0.8,0.6,0.3,1));
	GLfloat dir = cpBodyGetAngle(tank->data.body)*(180/M_PI);
	GLfloat rot = cpBodyGetAngle(tank->wheel1)*(180/M_PI);
	//glColor3f(1,0,0);
	//draw_simple_circle(temp->wheel1->p.x,temp->wheel1->p.y,15,rot);
	//glColor3f(1,0,0);
	//draw_simple_circle(temp->wheel2->p.x,temp->wheel2->p.y,15,rot);

	cpVect r = cpvadd(tank->data.body->p, cpvmult(cpvforangle(tank->angle),80));
	draw_line(tank->data.body->p.x,tank->data.body->p.y,r.x,r.y, 30);

	hpbar_draw(&tank->hp_bar);

	draw_texture(tank->param->tex_id, &(tank->data.body->p), &tex_map[0],200, 100, dir);
	draw_texture(tank->param->tex_id, &tank->wheel1->p, &tex_map[1],100, 100, rot);
	draw_texture(tank->param->tex_id, &tank->wheel2->p, &tex_map[1],100, 100, rot);
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	object_group_tank *tank = ((object_group_tank *)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->alive = 0;


	particles_get_emitter_at(EMITTER_EXPLOTION, b->body->p);

	//TODO create a function for damaging other objects
	tank->hp_bar.value -= 10;

	if (tank->hp_bar.value <= 0) {
		//a->body->data = NULL;
		if (tank->data.alive) {
			particles_get_emitter_at(EMITTER_EXPLOTION, b->body->p);
			particles_add_score_popup(b->body->p, tank->param->score);

			//TODO create a function for both adding score and creating score pop-up emitters
			((object_group_player *) objects_first(ID_PLAYER))->score +=
					tank->param->score;
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
