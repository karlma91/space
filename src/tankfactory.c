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


static void init(object *fac);
static void update(object *fac);
static void render(object *fac);
static void destroy(object *obj);

struct obj_type type_tank_factory = {
		0,
		init,
		update,
		render,
		destroy
};

struct factory {
	cpBody *body;
	cpShape *shape;
	int max;
	cpFloat timer;
	int cur;
	int max_hp;
	int hp;
	int x_pos;
};

static struct factory *temp;

void tankfactory_init(object *obj, int x_pos , int max_tanks, int max_hp)
{
	obj->type = &type_tank_factory;

	struct factory *fac = malloc(sizeof(struct factory));
	fac->max = max_tanks;
	fac->max_hp = max_hp;
	fac->hp = max_hp;

	fac->x_pos = x_pos;

	cpFloat size = 100;
	/* make and add new body */
	fac->body = cpSpaceAddBody(space, cpBodyNew(100, cpMomentForBox(10.0f, size, size)));
	cpBodySetPos(fac->body, cpv(x_pos,size));
	cpBodySetUserData(fac->body, obj);
	/* make and connect new shape to body */
	fac->shape = cpSpaceAddShape(space, cpBoxShapeNew(fac->body, size, size));
	cpShapeSetFriction(fac->shape, 1);
	cpShapeSetUserData(fac->shape, draw_boxshape);


	obj->data = fac;

}


static void init(object *fac)
{
	temp = ((struct factory*)fac->data);
}

static void update(object *fac)
{
	temp = ((struct factory*)fac->data);
	temp->timer+=dt;

}

static void render(object *fac)
{
	temp = ((struct factory*)fac->data);

	glColor3f(1,0,0);
	font_drawText(temp->shape->body->p.x,temp->shape->body->p.y,"TANK FACTORY");
	glColor3f(1,1,0);
	draw_boxshape(temp->shape);
}

static void destroy(object *obj)
{
	free(obj->data);
}
