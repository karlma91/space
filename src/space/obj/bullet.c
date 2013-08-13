/* header */
#include "objects.h"

#include "../game.h"
#include "../spaceengine.h"
#include "../states/space.h"
#include "../../engine/engine.h"


static void init(instance *obj);
static void update(struct bullet *);
static void render(instance *obj);
static void destroy(struct bullet *);

static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

//TODO standardize bullet
instance *object_create_bullet(cpVect pos, cpVect dir, cpVect intit_vel, int type)
{
	struct bullet *temp = (struct bullet *)objects_super_malloc(type, sizeof(struct bullet));
		temp->data.alive = 1;
		temp->data.components.damage = &(temp->damage);
		temp->data.components.body_count = 0;

		sprite_create(&(temp->data.spr), SPRITE_GLOW_DOT, 30, 30, 0);

		cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

		temp->data.body = cpSpaceAddBody(space, cpBodyNew(1, moment));
		cpBodySetPos(temp->data.body, cpvadd(pos, cpvmult(dir,30)));
		cpBodySetUserData(temp->data.body, (instance*)temp);
		cpBodySetVel(temp->data.body,cpvadd(cpvmult(dir,1500),intit_vel)); //3000
		temp->data.body->velocity_func = bulletVelocityFunc;

		temp->shape =se_add_circle_shape(temp->data.body,15,1,0);

		// Sets bullets collision type
		cpShapeSetCollisionType(temp->shape, type);
		cpShapeSetGroup(temp->shape,10);

		if(type == ID_BULLET_PLAYER){
			temp->data.preset = &type_bullet_player;
			cpShapeSetLayers(temp->shape,LAYER_PLAYER_BULLET);
			temp->damage = 32;
			if (keys[SDL_SCANCODE_RSHIFT])
				temp->damage = 200000;
		}else{
			temp->data.preset = &type_bullet_enemy;
			cpShapeSetLayers(temp->shape,LAYER_ENEMY_BULLET);
			temp->damage = 10;
		}

		temp->energy = 750; // number of msec energy

		objects_add((instance*)temp);
		return (instance*)temp;
}

static void init(instance *obj)
{

}

static void update(struct bullet *bullet)
{
	if (bullet->energy < 0) {
		bullet->data.alive = 0;
	} else {
		bullet->energy -= mdt;
	}
}

static void render(instance *obj)
{

	struct bullet *temp = (struct bullet*)obj;

	float alpha = temp->energy < 500 ? temp->energy / 500 : 1 ;

	if(temp->data.preset->ID == ID_BULLET_PLAYER){
		draw_color4f(0.3,0.3,0.9,alpha);
	}else{
		draw_color4f(0.9,0.3,0.3,alpha);
	}

	{
		cpShape *shape = temp->shape;

		cpCircleShape *circle = (cpCircleShape *)shape;
		cpVect vel = cpBodyGetVel(cpShapeGetBody(shape));

		cpVect pos_from = circle->tc;
		cpVect pos_to = circle->tc;
		pos_to.x -= vel.x/128;
		pos_to.y -= vel.y/128;

#if EXPERIMENTAL_GRAPHICS
		se_rect2arch(&pos_from);
		se_rect2arch(&pos_to);
#endif
		draw_glow_line(pos_from.x, pos_from.y, pos_to.x, pos_to.y, 64); //40 = 4 * radius
	}
}


/**
 * Velocity function to remove gravity
 */
static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpVect g = cpv(0, 0); //-1000 //200

	cpBodyUpdateVelocity(body, g, damping, dt);
}

static void destroy(struct bullet *bullet)
{
	cpSpaceRemoveShape(space, bullet->shape);
	cpSpaceRemoveBody(space, bullet->data.body);
	cpShapeFree(bullet->shape);
	cpBodyFree(bullet->data.body);
	objects_super_free((instance *)bullet);
	bullet = NULL;
}
