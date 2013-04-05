#include "chipmunk.h"
#include "space.h"
#include "objects.h"
#include "particles.h"
#include "player.h"
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

/**
 * returns the best angle to shoot at a moving object obj2 from obj1
 */
cpFloat se_get_best_shoot_angle(cpBody *a, cpBody *b, cpFloat bullet_speed)
{
	cpVect v = cpvsub(a->p, b->p);

	cpFloat c = cpvlength(b->v);
	cpFloat s = bullet_speed;
	cpFloat G = acos(cpvdot(v, b->v)/(cpvlength(b->v) * cpvlength(v)));
	cpFloat angle = asin((c * sin(G)) / s);

	cpFloat bc = cpvtoangle(v);

	if(b->v.x < 0){
		angle  = -angle;
	}
	angle  = M_PI + (bc - angle);

	return angle;
}

cpFloat get_angle(cpBody *a, cpBody *b)
{
	cpVect v = cpvsub(a->p, b->p);
	cpFloat bc = cpvtoangle(v);
	return bc;
}

void se_add_explotion_at_contact_point(cpArbiter *arb)
{
	cpVect v = cpArbiterGetPoint(arb, 0);
	particles_get_emitter_at(EMITTER_EXPLOSION, v);
}


static void add_shape(cpShape *shape, cpFloat friction, cpFloat elasticity)
{
	cpSpaceAddShape(space, shape);
	cpShapeSetFriction(shape, friction);
	cpShapeSetElasticity(shape, elasticity);
}

