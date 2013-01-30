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

struct tank_factory {
	int max;
	cpFloat timer;
	int cur;
	int max_hp;
	int hp;
	int x_pos;
};

void tankfactory_init(object *obj, int x_pos , int max_tanks, int max_hp)
{
	struct tank_factory *fac = malloc(sizeof(struct tank_factory));
	fac->max = max_tanks;
	fac->max_hp = max_hp;
	fac->hp = max_hp;

	fac->x_pos = x_pos;

	cpFloat size = 100;
	/* make and add new body */
	obj->body = cpSpaceAddBody(space, cpBodyNew(100, cpMomentForBox(10.0f, size, size)));
	cpBodySetPos(obj->body, cpv(x_pos,size));
	/* make and connect new shape to body */
	obj->shape = cpSpaceAddShape(space, cpBoxShapeNew(obj->body, size, size));
	cpShapeSetFriction(obj->shape, 1);

	obj->data = fac;

	obj->init = init;
	obj->update = update;
	obj->render = render;
	obj->destroy = destroy;

}


static void init(object *fac)
{

}

static void update(object *fac)
{

}

static void render(object *fac)
{
	glColor3f(1,0,0);
	font_drawText(fac->shape->body->p.x,fac->shape->body->p.y,"TANK FACTORY");
	glColor3f(1,1,0);
	draw_boxshape(fac->shape);
}

static void destroy(object *obj)
{
	free(obj->data);
}
