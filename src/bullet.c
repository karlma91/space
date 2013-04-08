/* header */
#include "bullet.h"
#include "objects.h"
#include "spaceengine.h"

static void init(object_data *obj);
static void update(object_data *obj);
static void render(object_data *obj);
static void destroy(object_data *obj);

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


//TODO standardize bullet
object_data *object_create_bullet(cpVect pos, cpVect dir, cpVect intit_vel, int type)
{
	struct bullet *temp = malloc(sizeof(struct bullet));

		temp->data.alive = 1;
		temp->data.components.damage = &(temp->damage);

		cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

		temp->data.body = cpSpaceAddBody(space, cpBodyNew(1, moment));
		cpBodySetPos(temp->data.body, cpvadd(pos, cpvmult(dir,60)));
		cpBodySetUserData(temp->data.body, (object_data*)temp);
		cpBodySetVel(temp->data.body,cpvadd(cpvmult(dir,3000),intit_vel));
		temp->data.body->velocity_func = bulletVelocityFunc;

		temp->shape =se_add_circle_shape(temp->data.body,15,1,0);

		// Sets bullets collision type
		cpShapeSetCollisionType(temp->shape, type);
		cpShapeSetGroup(temp->shape,10);

		if(type == ID_BULLET_PLAYER){
			temp->data.preset = &type_bullet_player;
			cpShapeSetLayers(temp->shape,LAYER_PLAYER_BULLET);
			temp->damage = 20;
		}else{
			temp->data.preset = &type_bullet_enemy;
			cpShapeSetLayers(temp->shape,LAYER_ENEMY_BULLET);
			temp->damage = 10;
		}

		objects_add((object_data*)temp);
		return (object_data*)temp;
}

static void init(object_data *obj)
{

}

static void update(object_data *obj)
{
	struct bullet *temp = (struct bullet*)obj;
}

static void render(object_data *obj)
{
	struct bullet *temp = (struct bullet*)obj;
	if(temp->data.preset->ID == ID_BULLET_PLAYER){
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


/**
 * Velocity function to remove gravity
 */
static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpVect g = cpv(0,-100);

	cpBodyUpdateVelocity(body, g, damping, dt);
}

static void destroy(object_data *bullet)
{
	*bullet->remove = 1;
	struct bullet *temp = (struct bullet*)bullet;
	cpSpaceRemoveShape(space, temp->shape);
	cpSpaceRemoveBody(space, temp->data.body);
	cpShapeFree(temp->shape);
	cpBodyFree(temp->data.body);
	free(temp);
	temp = NULL;
}
