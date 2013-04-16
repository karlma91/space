/* header */
#include "rocket.h"

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
#include "collisioncallbacks.h"

/* static prototypes */
static void init(object_group_rocket *);
static void update(object_group_rocket *);
static void render(object_group_rocket *);
static void destroy(object_group_rocket *);

object_group_preset type_rocket= {
	ID_ROCKET,
	init,
	update,
	render,
	destroy
};

float damage = 50;

object_group_rocket *object_create_rocket(float xpos, object_group_factory *factory, object_param_rocket *param)
{
	//TODO use pointer value as group id
	object_group_rocket *rocket = malloc(sizeof(*rocket));
	rocket->data.preset = &type_rocket;
	rocket->data.components.hp_bar = &(rocket->hp_bar);
	rocket->data.components.score = &(param->score);
	rocket->data.components.damage = &(damage);
	rocket->data.alive = 1;
	rocket->param = param;
	rocket->flame = particles_get_emitter(EMITTER_ROCKET_FLAME);
	rocket->active = 0;
	rocket->factory = factory;
	rocket->angle = M_PI/2;

	cpFloat start_height;
	cpFloat height = 30;
	cpFloat mass = 2.0f;
	cpFloat width = 30;
	if (factory){
		rocket->factory_id = factory->data.instance_id;
		start_height = factory->data.body->p.y;
	}else{
		start_height = 200 + 100;
	}


	// Make a car with some nice soft suspension
	cpVect boxOffset = cpv(0, 0);


	rocket->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPos(rocket->data.body , cpvadd(cpv(xpos, start_height), boxOffset));
	cpBodySetVelLimit(rocket->data.body,1000);
	rocket->shape = se_add_box_shape(rocket->data.body,width,height,0.7,0.0);

	//cpShapeSetGroup(tempShape, 1); // use a group to keep the car parts from colliding

	cpShapeSetLayers(rocket->shape, LAYER_TANK);
	cpShapeSetCollisionType(rocket->shape, ID_ROCKET);

	cpBodySetUserData(rocket->data.body, rocket);
	objects_add((object_data *)rocket);

	hpbar_init(&rocket->hp_bar, param->max_hp, 80, 16, -40, 60, &(rocket->data.body->p));

	return rocket;
}


static void init(object_group_rocket *tank)
{
}

static void update(object_group_rocket *rocket)
{
	rocket->timer +=dt;

	rocket->flame->p = rocket->data.body->p;
	rocket->flame->angular_offset = cpvtoangle(rocket->data.body->v) * (180/M_PI)+90;

	/* gets the player from the list */
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

	cpVect pl = player->data.body->p;
	cpVect rc = rocket->data.body->p;

	if(rc.y>currentlvl->height*0.35){
		rocket->active = 1;
	}

	cpFloat ptx = (pl.x-rc.x); //direct way
	cpFloat pltx = (rc.x - currentlvl->left + (currentlvl->right - pl.x));
	cpFloat prtx = (currentlvl->right - rc.x + (pl.x - currentlvl->left));
	if(fabs(ptx) < prtx && fabs(ptx) < pltx){
		ptx = ptx>0? 1:-1;
	}else if(pltx < prtx){
		pl.x -= currentlvl->right - currentlvl->left;
		ptx = -1;
	}else {
		pl.x += currentlvl->right - currentlvl->left;
		ptx = 1;
	}

	float target_angle;

	cpBodySetForce(rocket->data.body,cpvzero);
	if(rocket->active){
		target_angle = get_angle(pl,rc);
		cpBodyApplyForce(rocket->data.body,cpvmult(cpvforangle(target_angle),rocket->param->force),cpvzero);
	}else{
		target_angle = M_PI / 2;
		cpBodyApplyForce(rocket->data.body,cpvmult(cpv(0,1),rocket->param->force),cpvzero);
	}

	rocket->angle = turn_toangle(rocket->angle, target_angle,1 * (2*M_PI* dt));
}

static void render(object_group_rocket *rocket)
{
	if (rocket->param->max_hp < 100)
		glColor4f(1,1,1,0.6);
	else
		glColor4f(1,1,0,0.6);

	glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	hpbar_draw(&rocket->hp_bar);

	int texture = rocket->param->tex_id;

	draw_texture(texture, &(rocket->data.body->p),TEX_MAP_FULL,100, 100, rocket->angle*(180/M_PI));

	glPopAttrib();
}


static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}

static void destroy(object_group_rocket *rocket)
{
	cpBodyEachShape(rocket->data.body,shape_from_space,NULL);

	cpSpaceRemoveBody(space, rocket->data.body);
	cpBodyFree(rocket->data.body);

	particles_release_emitter(rocket->flame);
	if(rocket->factory != NULL){
		rocket->factory->cur--;
	}

	objects_remove((object_data *)rocket);
	free(rocket);
}
