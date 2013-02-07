/* header */
#include "bullet.h"

static void init(object *obj);
static void update(object *obj);
static void render(object *obj);
static void destroy(object *obj);

static int callback_ground(cpArbiter *arb, cpSpace *space, void *unused);

struct obj_type type_bullet_player= {
	ID_BULLET_PLAYER,
	init,
	update,
	render,
	destroy
};
struct obj_type type_bullet_enemy= {
	ID_BULLET_ENEMY,
	init,
	update,
	render,
	destroy
};

struct bullet *temp;

static void init(object *obj)
{

}
static void update(object *obj)
{
	temp = (struct bullet*)obj;
}
static void render(object *obj)
{
	temp = (struct bullet*)obj;
	if(temp->type->ID == ID_BULLET_PLAYER){
		glColor3f(0.9,0.3,0.3);
	}else{
		glColor3f(0.3,0.3,0.9);
	}
	draw_ballshape(temp->shape);
}

object *bullet_init(cpVect pos, cpVect dir, int type)
{
		temp = malloc(sizeof(struct bullet));
		temp->alive = 1;

		cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

		temp->body = cpSpaceAddBody(space, cpBodyNew(1, moment));
		cpBodySetPos(temp->body, cpvadd(pos, cpvmult(dir,15)));
		cpBodySetUserData(temp->body, (object*)temp);
		cpBodySetVel(temp->body,cpvmult(dir,1000));

		temp->shape = cpSpaceAddShape(space, cpCircleShapeNew(temp->body, 5, cpvzero));
		cpShapeSetFriction(temp->shape, 0.7);
		// Sets bullets collision type
		cpShapeSetCollisionType(temp->shape, type);
		cpSpaceAddCollisionHandler(space, ID_GROUND, type, callback_ground, NULL, NULL, NULL, NULL);
		cpShapeSetGroup(temp->shape,10);
		if(type == ID_BULLET_PLAYER){
			temp->type = &type_bullet_player;
			cpShapeSetLayers(temp->shape,LAYER_PLAYER_BULLET);
		}else{
			temp->type = &type_bullet_enemy;
			cpShapeSetLayers(temp->shape,LAYER_ENEMY_BULLET);
		}

		objects_add((object*)temp);
		return (object*)temp;
}


// from chipmunk docs
static int callback_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	temp = (struct bullet*)b->body->data;
	particles_add_explosion(cpBodyGetPos(cpShapeGetBody(b)), 0.3f, 1200, 20,300);
	temp->alive = 0;
	return 0;
}

static void destroy(object *bullet)
{
	temp = (struct bullet*)bullet;
	cpSpaceRemoveBody(space, temp->body);
	cpSpaceRemoveShape(space, temp->shape);
	*temp->remove = 1;
	free(temp);
	temp = NULL;
}
