/* header */
#include "tankfactory.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Game state */
#include "space.h"

/* Drawing */
#include "draw.h"
#include "particles.h"


#include "objects.h"
/* Game components */
#include "player.h"
#include "tank.h"
#include "bullet.h"

static void init(object *fac);
static void update(object *fac);
static void render(object *fac);
static void destroy(object *obj);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);
static void remove_factory_from_tank(object *obj);


struct obj_type type_tank_factory = {
	ID_TANK_FACTORY,
	init,
	update,
	render,
	destroy
};

static struct tank_factory *temp; //TODO fjerne denne!?

object *tankfactory_init( int x_pos , struct tank_factory_param *param)
{
	struct tank_factory *fac = malloc(sizeof(struct tank_factory));
	((object*)fac)->alive = 1;
	((object*)fac)->type = &type_tank_factory;
	fac->param = param;


	fac->cur = 0;

	fac->timer = (fac->param->spawn_delay)*0.7;
	fac->hp = fac->param->max_hp;

	cpFloat size = 100;
	/* make and add new body */
	((object*)fac)->body = cpSpaceAddBody(space, cpBodyNew(500, cpMomentForBox(500.0f, size, size)));
	cpBodySetPos(((object*)fac)->body, cpv(x_pos,size));

	/* make and connect new shape to body */
	fac->shape = cpSpaceAddShape(space, cpBoxShapeNew(((object*)fac)->body, size, size));
	cpShapeSetFriction(fac->shape, 1);

	//cpShapeSetGroup(fac->shape, 10);

	cpShapeSetLayers(fac->shape,LAYER_TANK_FACTORY);

	cpShapeSetCollisionType(fac->shape, ID_TANK_FACTORY);
	cpSpaceAddCollisionHandler(space, ID_TANK_FACTORY, ID_BULLET_PLAYER, collision_player_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(((object*)fac)->body, (object*)fac);
	objects_add((object*)fac);
	return (object*)fac;
}


static void init(object *fac)
{
	temp = ((struct tank_factory*)fac);
}

static void update(object *fac)
{
	temp = ((struct tank_factory*)fac);
	temp->timer+=dt;
	if(temp->timer > temp->param->spawn_delay && temp->cur < temp->param->max_tanks){
		temp->timer = 0;
		tank_init(fac->body->p.x, temp, temp->param->t_param );
		temp->cur += 1;
	}
}

static void render(object *fac)
{
	temp = ((struct tank_factory*)fac);

	//glColor3f(1,1,1);
	draw_hp(fac->body->p.x-50, fac->body->p.y + 90, 100, 16, temp->hp / temp->param->max_hp);


	if (temp->param->max_hp < 300)
		glColor3f(0.5,0.8,0.9);
	else
		glColor3f(0.9,0.5,0.5);

	//draw_boxshape(temp->shape,RGBAColor(0.2,0.9,0.1,1),RGBAColor(0.6,0.9,0.4,1));
	temp->rot += 381*dt;

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(fac->body->p.x,fac->body->p.y, 0.0f);
	glRotatef(temp->rot,0,0,1);
	glScalef(150,150,1);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2d(0, 0); glVertex2d(-0.5, -0.5);
	glTexCoord2d(0, 1); glVertex2d(-0.5, 0.5);
	glTexCoord2d(1.0f, 0); glVertex2d(0.5, -0.5);
	glTexCoord2d(1.0f, 1); glVertex2d(0.5, 0.5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(fac->body->p.x,fac->body->p.y, 0.0f);
	glScalef(200,200,1);
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2d(0, 1); glVertex2d(-0.5, -0.5);
	glTexCoord2d(0, 0); glVertex2d(-0.5, 0.5);
	glTexCoord2d(1.0f, 1); glVertex2d(0.5, -0.5);
	glTexCoord2d(1.0f, 0); glVertex2d(0.5, 0.5);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);


}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);

	temp = ((struct tank_factory*)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->alive = 0;

	particles_add_explosion(b->body->p,0.3,1500,15,200);

	temp->hp -= 10;
	if(temp->hp <=0 ){
		particles_add_explosion(a->body->p,1,2000,50,800);
		if(((object *) temp)->alive){
			((struct player *)objects_first(ID_PLAYER))->score += temp->param->score;
		}
		((object*)temp)->alive = 0;
		objects_iterate_type(remove_factory_from_tank,ID_TANK);
	}

	return 0;
}

static void remove_factory_from_tank(object *obj)
{
	struct tank *t = (struct tank *)obj;
	if(t->factory == temp){
		t->factory = NULL;
	}
}


static void destroy(object *obj)
{
	temp = ((struct tank_factory*)obj);
	*obj->remove = 1;

	cpSpaceRemoveBody(space, obj->body);
	cpSpaceRemoveShape(space, temp->shape);
	free(obj);
}
