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

/* helper */
static float get_angle(object *obj, object *obj2);
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

	cpFloat size = 50;
	/* make and add new body */
	((object *) tank)->body = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(20.0f, size, size)));
	cpBodySetPos(((object *) tank)->body, cpv(xpos,size+10));
	cpBodySetVelLimit(((object *) tank)->body,180);
	/* make and connect new shape to body */
	tank->shape = cpSpaceAddShape(space, cpBoxShapeNew(((object *) tank)->body, size, size));
	cpShapeSetFriction(tank->shape, 0.01);
	//cpShapeSetGroup(tank->shape, 10);
	cpShapeSetLayers(tank->shape,LAYER_TANK);
	cpShapeSetCollisionType(tank->shape, ID_TANK);
	cpSpaceAddCollisionHandler(space, ID_TANK, ID_BULLET_PLAYER, collision_player_bullet, NULL, NULL, NULL, NULL);

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

	float player_angle = get_best_angle(fac, ((object*)player));


	/*TODO: stop shaking when at correct angle */
		/* som i matlab */
		temp->angle += ((player_angle > temp->angle)*2 - 1) * temp->rot_speed;

	if(temp->timer > 2 + ((3.0f*rand())/RAND_MAX)){
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

	cpBodySetForce(fac->body,cpv(ptx*12000,0));

}

static float get_best_angle(object *obj, object *obj2)
{
	cpVect a = cpvsub(obj->body->p, obj2->body->p);

	cpFloat c = cpvlength(obj2->body->v);
	cpFloat b = 1000;
	cpFloat G = acos(cpvdot(a,obj2->body->v)/(cpvlength(obj2->body->v)*cpvlength(a)));
	float angle = asin((c*sin(G))/b);

	cpFloat bc = cpvtoangle(a);

	if(player->body->v.x < 0){
		angle  = -angle;
	}
	angle  = M_PI + (bc  - angle );

	return angle;
}

static float get_angle(object *obj, object *obj2)
{
	cpVect a = cpvsub(obj->body->p, obj2->body->p);
	cpFloat bc = cpvtoangle(a);
	return bc;
}

static void render(object *fac)
{
	temp = ((struct tank*)fac);

	glColor3f(1,1,1);
	draw_hp(fac->body->p.x-50, fac->body->p.y + 60, 100, 20, temp->hp / temp->param->max_hp);
	glColor3f(1,1,0);
	draw_boxshape(temp->shape,RGBAColor(0.8,0.3,0.1,1),RGBAColor(0.8,0.6,0.3,1));

	cpVect r = cpvadd(fac->body->p, cpvmult(cpvforangle(temp->angle),60));
	draw_line(fac->body->p.x,fac->body->p.y,r.x,r.y, 30);

}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	temp = ((struct tank*)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->alive = 0;

	particles_add_explosion(b->body->p,0.3,1500,10,200);

	if(temp->hp <=0 ){
		//a->body->data = NULL;
		particles_add_explosion(a->body->p,1,2000,50,800);
		//cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
		((object *) temp)->alive = 0;
	}else{
		temp->hp -= 10;
	}

	return 0;
}


static void destroy(object *obj)
{
	temp = ((struct tank*)obj);
	cpSpaceRemoveBody(space, obj->body);
	cpSpaceRemoveShape(space, temp->shape);

	if(temp->factory != NULL){
		temp->factory->cur--;
	}

	*obj->remove = 1;
	free(obj);
}
