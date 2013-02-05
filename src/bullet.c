/* header */
#include "bullet.h"

static void init(object *obj);
static void update(object *obj);
static void render(object *obj);
static void destroy(object *obj);

static int callback_ground(cpArbiter *arb, cpSpace *space, void *unused);

struct obj_type type_bullet= {
	ID_BULLET,
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
	if (temp->body->p.x < level_left + 50) temp->body->p.x = level_right - 50;
	if (temp->body->p.x > level_right - 50) temp->body->p.x = level_left + 50;
}
static void render(object *obj)
{
	temp = (struct bullet*)obj;
	draw_ballshape(temp->shape);
}

object *bullet_init(cpVect pos, cpVect dir, int type)
{
		temp = malloc(sizeof(struct bullet));
		temp->alive = 1;
		temp->type = &type_bullet;

		cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

		temp->body = cpSpaceAddBody(space, cpBodyNew(1, moment));
		cpBodySetPos(temp->body, cpvadd(pos, cpvmult(dir,5)));
		cpBodySetUserData(temp->body, (object*)temp);
		cpBodySetVel(temp->body,cpvmult(dir,1500));

		temp->shape = cpSpaceAddShape(space, cpCircleShapeNew(temp->body, 5, cpvzero));
		cpShapeSetFriction(temp->shape, 0.7);
		// Sets bullets collision type
		cpShapeSetCollisionType(temp->shape, ID_BULLET);
		// runs callback begin when bullet (2) hits ground (1)
		// this will make bullet b and ground a in begin callback
		cpSpaceAddCollisionHandler(space, ID_GROUND, ID_BULLET, callback_ground, NULL, NULL, NULL, NULL);

		list_add((object*)temp);
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
