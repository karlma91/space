/* header */
#include "bullet.h"
#include "objects.h"

static void init(object_data *obj);
static void update(object_data *obj);
static void render(object_data *obj);
static void destroy(object_data *obj);

static void callback_ground(cpArbiter *arb, cpSpace *space, void *unused);
static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

object_group_preset type_bullet_player= {
	ID_BULLET_PLAYER,
	init,
	update,
	render,
	destroy
};
object_group_preset type_bullet_enemy= {
	ID_BULLET_ENEMY,
	init,
	update,
	render,
	destroy
};

static struct bullet *temp;

static void init(object_data *obj)
{

}

static void update(object_data *obj)
{
	temp = (struct bullet*)obj;
}

static void render(object_data *obj)
{
	temp = (struct bullet*)obj;
	if(temp->type->ID == ID_BULLET_PLAYER){
		glColor3f(0.9,0.3,0.3);
	}else{
		glColor3f(0.3,0.3,0.9);
	}

	{
		cpShape *shape = temp->shape;

		cpCircleShape *circle = (cpCircleShape *)shape;
		cpVect vel = cpBodyGetVel(cpShapeGetBody(shape));
		draw_line(circle->tc.x, circle->tc.y, circle->tc.x - vel.x/128, circle->tc.y - vel.y/128, 64); //40 = 4 * radius
	}
}

//TODO standardize bullet
object_data *bullet_init(cpVect pos, cpVect dir, cpVect intit_vel, int type)
{
		temp = malloc(sizeof(struct bullet));
		temp->alive = 1;

		cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

		temp->body = cpSpaceAddBody(space, cpBodyNew(1, moment));
		cpBodySetPos(temp->body, cpvadd(pos, cpvmult(dir,60)));
		cpBodySetUserData(temp->body, (object_data*)temp);
		cpBodySetVel(temp->body,cpvadd(cpvmult(dir,3000),intit_vel));
		temp->body->velocity_func = bulletVelocityFunc;

		temp->shape = cpSpaceAddShape(space, cpCircleShapeNew(temp->body, 15, cpvzero));
		cpShapeSetFriction(temp->shape, 1.0);
		// Sets bullets collision type
		cpShapeSetCollisionType(temp->shape, type);
		cpSpaceAddCollisionHandler(space, ID_GROUND, type, NULL, NULL, callback_ground, NULL, NULL);
		cpShapeSetGroup(temp->shape,10);
		if(type == ID_BULLET_PLAYER){
			temp->type = &type_bullet_player;
			cpShapeSetLayers(temp->shape,LAYER_PLAYER_BULLET);
			temp->damage = 20;
		}else{
			temp->type = &type_bullet_enemy;
			cpShapeSetLayers(temp->shape,LAYER_ENEMY_BULLET);
			temp->damage = 10;
		}

		objects_add((object_data*)temp);
		return (object_data*)temp;
}

/**
 * Velocity function to remove gravity
 */
static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpVect g = cpv(0,-100);

	cpBodyUpdateVelocity(body, g, damping, dt);
}

// from chipmunk docs
static void callback_ground(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	temp = (struct bullet*)b->body->data;
	cpVect v = cpArbiterGetPoint(arb, 0);
	cpVect n = cpArbiterGetNormal(arb, 0);
	n=cpvneg(n);
	float angle = cpvtoangle(n);
	cpVect force = cpArbiterTotalImpulseWithFriction(arb);
	float f = cpvlength(force);
	particles_add_sparks(v,angle,f);
	temp->alive = 0; // TODO FIXME BUG: segmentation fault?
}

static void destroy(object_data *bullet)
{
	temp = (struct bullet*)bullet;
	cpSpaceRemoveShape(space, temp->shape);
	cpSpaceRemoveBody(space, temp->body);
	cpShapeFree(temp->shape);
	cpBodyFree(temp->body);
	*temp->remove = 1;
	free(temp);
	temp = NULL;
}
