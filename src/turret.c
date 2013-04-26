/* header */
#include "turret.h"

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
static void init(object_group_turret *);
static void update(object_group_turret *);
static void render(object_group_turret *);
static void destroy(object_group_turret *);
static void velfunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

object_group_preset type_turret= {
	ID_TURRET,
	init,
	update,
	render,
	destroy
};

static const texture_map tex_map[2] = {
		{0,1, 0.5,1, 0,0, 0.5,0}, {0.5,1, 1,1, 0.5,0, 1,0}
};

object_group_turret *object_create_turret(float xpos, object_param_turret *param)
{
	//TODO use pointer value as group id
	object_group_turret *turret = malloc(sizeof(*turret));
	turret->data.preset = &type_turret;
	turret->data.components.hp_bar = &(turret->hp_bar);
	turret->data.components.score = &(param->score);
	turret->data.components.body_count = 0;
	turret->data.alive = 1;
	turret->param = param;
	turret->timer = 0;
	turret->rate = 0.060;
	turret->bullets = 0;
	turret->barrel_angle = 3*(M_PI/2);
	turret->max_distance = 800;

	cpFloat size = 100;
	turret->data.body = cpSpaceAddBody(space,
			cpBodyNew(500, cpMomentForBox(5000.0f, size, size)));
	cpBodySetPos(turret->data.body, cpv(xpos,currentlvl->height - size/2));

	turret->data.body->velocity_func = velfunc;

	/* make and connect new shape to body */
	turret->shape = cpSpaceAddShape(space,cpBoxShapeNew(turret->data.body, size, size));
	cpShapeSetFriction(turret->shape, 1);

	//cpShapeSetGroup(fac->shape, 10);

	cpShapeSetLayers(turret->shape, LAYER_TANK_FACTORY);

	cpShapeSetCollisionType(turret->shape, ID_FACTORY);


	cpBodySetUserData(turret->data.body, turret);
	objects_add((object_data *)turret);

	hpbar_init(&turret->hp_bar,param->max_hp,80,16,-40,60,&(turret->data.body->p));

	return turret;
}


static void init(object_group_turret *turret)
{
}

static void update(object_group_turret *turret)
{
	/* gets the player from the list */
	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

	cpVect pl = player->data.body->p;
	cpVect rc = turret->data.body->p;

	cpFloat best_angle = se_get_best_shoot_angle(rc,turret->data.body->v, pl, player->data.body->v, 3000);

	best_angle = best_angle - cpvtoangle(turret->data.body->rot);
	if(best_angle < 0){
		best_angle += 2*M_PI;
	}else if(best_angle>2*M_PI){
		best_angle -= 2*M_PI;
	}
	turret->barrel_angle=turn_toangle(turret->barrel_angle, best_angle, turret->param->rot_speed * dt);

	if(turret->timer > turret->param->shoot_interval){
		turret->shooting = 1;
		turret->timer = 0;
	}
	if(turret->shooting && turret->timer > turret->rate && se_distance_to_player(turret->data.body->p.x) < turret->max_distance){
		turret->bullets += 1;
		cpVect shoot_angle = cpvforangle(turret->barrel_angle + cpBodyGetAngle(turret->data.body));
		object_create_bullet(turret->data.body->p,shoot_angle ,turret->data.body->v,ID_BULLET_ENEMY);
		if(turret->bullets > turret->param->burst_number){
			turret->bullets = 0;
			turret->shooting = 0;
		}
		turret->timer = 0;
	}else{
		turret->timer +=dt;
	}

}

static void render(object_group_turret *turret)
{

	draw_color4f(1,1,1,1);

	GLfloat dir = cpBodyGetAngle(turret->data.body);

	int texture = turret->param->tex_id;

	draw_texture(texture, &(turret->data.body->p), &tex_map[0],100, 100, dir*(180/M_PI));
	draw_texture(texture, &(turret->data.body->p), &tex_map[1],100, 100, turret->barrel_angle*(180/M_PI));

	hpbar_draw(&turret->hp_bar);
}


static void velfunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpVect g = cpv(0,100);

	cpBodyUpdateVelocity(body, g, damping, dt);
}

static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}

static void destroy(object_group_turret *turret)
{
	cpBodyEachShape(turret->data.body,shape_from_space,NULL);

	cpSpaceRemoveBody(space, turret->data.body);
	cpBodyFree(turret->data.body);

	objects_remove((object_data *)turret);
	free(turret);
}
