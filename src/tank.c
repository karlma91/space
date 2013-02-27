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
static void init(object *fac);
static void update(object *fac);
static void render(object *fac);
static void destroy(object *obj);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);
static cpShape *tempShape;
static cpBody *addChassis(cpSpace *space, cpVect pos, cpVect boxOffset);
static cpBody *addWheel(cpSpace *space, cpVect pos, cpVect boxOffset);

/* helper */
//static float get_angle(object *obj, object *obj2);
static float get_best_angle(object *obj, object *obj2);


struct obj_type type_tank= {
	ID_TANK,
	init,
	update,
	render,
	destroy
};


static struct tank *temp;

object *tank_init(float xpos,struct tank_factory *factory, struct tank_param *pram)
{
	struct tank *tank = malloc(sizeof(struct tank));
	((object *) tank)->type = &type_tank;
	((object *) tank)->alive = 1;
	tank->param = pram;

	tank->hp = tank->param->max_hp;
	tank->timer = 0;
	tank->factory = factory;

	tank->rot_speed = 0.01;

//	cpFloat width = 50;
	cpFloat height = 30;


	/* make and add new body */
	// Make a car with some nice soft suspension
	cpVect boxOffset;
	cpVect posA = cpv(xpos-20, 120);
	cpVect posB = cpv(xpos+20, 120);
	boxOffset = cpv(0, 0);

	((object *) tank)->body = addChassis(space, cpv(xpos, height+10),boxOffset);
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


	cpSpaceAddConstraint(space, cpGrooveJointNew(((object *) tank)->body, tank->wheel1 , cpv(-30, -10), cpv(-30, -40), cpvzero));
	cpSpaceAddConstraint(space, cpGrooveJointNew(((object *) tank)->body, tank->wheel2, cpv( 30, -10), cpv( 30, -40), cpvzero));

	cpSpaceAddConstraint(space, cpDampedSpringNew(((object *) tank)->body, tank->wheel1 , cpv(-30, 0), cpvzero, 50.0f, 60.0f, 0.5f));
	cpSpaceAddConstraint(space, cpDampedSpringNew(((object *) tank)->body, tank->wheel2, cpv( 30, 0), cpvzero, 50.0f, 60.0f, 0.5f));


	cpBodySetUserData(((object *) tank)->body, (object*)tank);
	objects_add((object*)tank);
	return (object*)tank;
}


static void init(object *fac)
{
	temp = ((struct tank*)fac);
}

static void update(object *fac)
{
	temp = ((struct tank*)fac);
	temp->timer +=dt;

	/* gets the player from the list */
	struct player *player = ((struct player*)objects_first(ID_PLAYER));

	float player_angle = get_best_angle(fac, ((object*)player));


	/*TODO: stop shaking when at correct angle */
		/* som i matlab */
		temp->angle += ((player_angle > temp->angle)*2 - 1) * temp->rot_speed;

	if(temp->timer > 1 + ((3.0f*rand())/RAND_MAX)){
		cpVect t = cpvforangle(temp->angle );
		bullet_init(fac->body->p,t,ID_BULLET_ENEMY);
		temp->timer = 0;
	}

	cpFloat tx = fac->body->p.x;
	cpFloat px = player->body->p.x;

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
		cpBodySetTorque(temp->wheel1,20000);
		cpBodySetTorque(temp->wheel2,20000);
	}else{
		cpBodySetTorque(temp->wheel1,-20000);
		cpBodySetTorque(temp->wheel2,-20000);
	}

}

/**
 * returns the best angle to shoot at a moving object obj2 from obj1
 */
static float get_best_angle(object *obj, object *obj2)
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

static void render(object *fac)
{
	temp = ((struct tank*)fac);

	draw_boxshape(temp->shape,RGBAColor(0.8,0.3,0.1,1),RGBAColor(0.8,0.6,0.3,1));
	GLfloat rot = cpBodyGetAngle(temp->wheel1)*(180/M_PI);
	glColor3f(1,0,0);
	draw_simple_circle(temp->wheel1->p.x,temp->wheel1->p.y,15,rot);
	glColor3f(1,0,0);
	draw_simple_circle(temp->wheel2->p.x,temp->wheel2->p.y,15,rot);

	cpVect r = cpvadd(fac->body->p, cpvmult(cpvforangle(temp->angle),60));
	draw_line(fac->body->p.x,fac->body->p.y,r.x,r.y, 30);

	draw_hp(fac->body->p.x-15, fac->body->p.y + 25, 30, 8, temp->hp / temp->param->max_hp);
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	temp = ((struct tank*)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->alive = 0;

	particles_add_explosion(b->body->p,0.3,1500,10,200);

	temp->hp -= 10;

	if(temp->hp <=0 ){
		//a->body->data = NULL;
		particles_add_explosion(a->body->p,1,2000,20,800);
		if(((object *) temp)->alive){
			((struct player *)objects_first(ID_PLAYER))->highscore += temp->param->score;
		}
		//cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
		((object *) temp)->alive = 0;
	}

	return 0;
}


static void destroy(object *obj)
{
	temp = ((struct tank*)obj);
	cpSpaceRemoveBody(space, obj->body);
	cpSpaceRemoveShape(space, temp->shape);

	cpSpaceRemoveBody(space, temp->wheel1);
	cpSpaceRemoveBody(space, temp->wheel2);

	if(temp->factory != NULL){
		temp->factory->cur--;
	}

	*obj->remove = 1;
	free(obj);
}
