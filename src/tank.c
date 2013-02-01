#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "player.h"
#include "menu.h"
#include "math.h"
#include "tankfactory.h"
#include "list.h"

static void init(object *fac);
static void update(object *fac);
static void render(object *fac);
static void destroy(object *obj);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);

struct obj_type type_tank= {
	ID_TANK,
	init,
	update,
	render,
	destroy
};

struct tank {
	struct obj_type *type;
	int id;
	int *remove;
	cpBody *body;
	cpShape *shape;
	float max_hp;
	float hp;
};

static struct tank *temp;

object *tank_init( int x_pos, float max_hp)
{

	struct tank *tank = malloc(sizeof(struct tank));
	tank->type = &type_tank;
	tank->max_hp = max_hp;
	tank->hp = tank->max_hp;

	cpFloat size = 50;
	/* make and add new body */
	tank->body = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(20.0f, size, size)));
	cpBodySetPos(tank->body, cpv(x_pos,size+10));

	/* make and connect new shape to body */
	tank->shape = cpSpaceAddShape(space, cpBoxShapeNew(tank->body, size, size));
	cpShapeSetFriction(tank->shape, 1);

	cpShapeSetCollisionType(tank->shape, ID_TANK);
	cpSpaceAddCollisionHandler(space, ID_TANK, ID_PLAYER_BULLET, collision_player_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(tank->body, (object*)tank);
	list_add((object*)tank);
	return (object*)tank;
}


static void init(object *fac)
{
	temp = ((struct tank*)fac);
}

static void update(object *fac)
{
	temp = ((struct tank*)fac);

}

static void render(object *fac)
{
	temp = ((struct tank*)fac);

	glColor3f(1,1,1);
	draw_hp(temp->body->p.x-50, temp->body->p.y + 60, 100, 20, temp->hp / temp->max_hp);
	glColor3f(1,1,0);
	draw_boxshape(temp->shape);
}

static void postStepRemove(cpSpace *space, cpShape *shape, void *unused)
{
	cpSpaceRemoveBody(space, shape->body);
	cpBodyFree(shape->body);

	cpSpaceRemoveShape(space, shape);
	cpShapeFree(shape);
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	temp = ((struct tank*)(a->body->data));
	temp->hp -= 10;
	if(temp->hp <=0 ){
		destroy(a->body->data);
		a->body->data = NULL;
		particles_add_explosion(a->body->p,1,2000,50,800);
		cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
	}
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, b, NULL);

	return 0;
}


static void destroy(object *obj)
{
	*obj->remove = 1;
	free(obj);
}
