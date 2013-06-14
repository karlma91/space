#include "chipmunk.h"
#include "space.h"
#include "objects.h"
#include "particles.h"
#include "player.h"

#include "waffle_utils.h"

static void add_shape(cpShape *shape, cpFloat friction, cpFloat elasticity);

cpShape *se_add_circle_shape(cpBody *body, cpFloat radius, cpFloat friction, cpFloat elasticity)
{
	cpShape *shape = cpCircleShapeNew(body, radius, cpvzero);
	add_shape(shape,friction,elasticity);
	return shape;
}

cpShape *se_add_box_shape(cpBody *body, cpFloat width, cpFloat height, cpFloat friction, cpFloat elasticity)
{
	cpShape *shape = cpBoxShapeNew(body, width, height);
	add_shape(shape,friction,elasticity);
	return shape;
}

void se_add_score_and_popup(cpVect p, int score)
{
	particles_add_score_popup(p, score);
	((object_group_player *) objects_first(ID_PLAYER))->score += score;
}

float se_distance_to_player(float x)
{
	object_group_player *player = ((object_group_player *) objects_first(ID_PLAYER));
	return fabs(player->data.body->p.x-x);
}

/**
 * return 1 if object is killed
 */
int se_damage_object(object_data *object, float damage)
{
	object->components.hp_bar->value -= damage;
	if (object->components.hp_bar->value <= 0) {
		object->destroyed = 0;
		return 1;
	}
	return 0;
}

/**
 * returns the best angle to shoot at a moving object obj2 from obj1
 */
cpFloat se_get_best_shoot_angle(cpVect a, cpVect va, cpVect b, cpVect vb, cpFloat bullet_speed)
{
	cpVect v = cpvsub(a, b);

	cpFloat c = cpvlength(vb);
	cpFloat s = bullet_speed;
	cpFloat G = acos(cpvdot(v, vb)/(cpvlength(vb) * cpvlength(v)));
	cpFloat angle = asin((c * sin(G)) / s);

	cpFloat bc = cpvtoangle(v);

	if(vb.x < 0){
		angle  = -angle;
	}
	angle  = M_PI + (bc - angle);

	return angle;
}

cpFloat get_angle(cpVect a, cpVect b)
{
	cpFloat bc = cpvtoangle(cpvsub(a,b));
	return bc;
}


static void add_shape(cpShape *shape, cpFloat friction, cpFloat elasticity)
{
	cpSpaceAddShape(space, shape);
	cpShapeSetFriction(shape, friction);
	cpShapeSetElasticity(shape, elasticity);
}

//TODO add preferred angle to handle situations with two possible solutions
//TODO move this method to objects.c?
float turn_toangle(float from_angle, float to_angle, float step_size)
{
	from_angle += from_angle >= (2*M_PI) ? -(2*M_PI) : from_angle < 0 ? (2*M_PI) : 0;

	if (to_angle < from_angle - step_size) {
		if ((from_angle - to_angle) < M_PI) {
			from_angle -= step_size;
		} else {
			if (2*M_PI - (from_angle - to_angle) < step_size) {
				from_angle = to_angle;
			} else {
				from_angle += step_size;
			}
		}
	} else if (to_angle > from_angle + step_size) {
		if ((to_angle - from_angle) < M_PI) {
			from_angle += step_size;
		} else {
			if (2*M_PI - (to_angle - from_angle) < step_size) {
				from_angle = to_angle;
			} else {
				from_angle -= step_size;
			}
		}
	} else {
		from_angle = to_angle;
	}

	//SDL_Log("angle: %0.4f\n",from_angle*180/M_PI);
	return from_angle;
}
void se_shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}
void se_constrain_from_space(cpBody *body, cpConstraint *constraint, void *data)
{
    cpSpaceRemoveConstraint(space, constraint);
    cpConstraintFree(constraint);
}

