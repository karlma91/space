#include "chipmunk.h"
#include "states/space.h"
#include "game.h"

#include "../engine/engine.h"
#include "../engine/io/waffle_utils.h"
#include "../engine/graphics/camera.h"
#include "../engine/graphics/particles.h"
#include "../engine/state/statesystem.h"
#include "spaceengine.h"

#include <float.h>
#include <signal.h>

static void space_vel_func(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
	gravity = cpvmult(cpvnormalize_safe(body->p), cpvlength(gravity));
	cpBodyUpdateVelocity(body, gravity, damping, dt);
}static void space_vel_neg_func(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
	gravity = cpvmult(cpvnormalize_safe(body->p), -cpvlength(gravity));
	cpBodyUpdateVelocity(body, gravity, damping, dt);
}
static void space_vel_func_zero_g(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
	cpBodyUpdateVelocity(body, cpvzero, damping, dt);
}

void se_add_score_and_popup(cpVect p, int score)
{
    particles_add_score_popup(RLAY_GAME_FRONT, p, score);
	((obj_player *) instance_first(obj_id_player))->coins += score;
}


float se_arcdist2player(cpVect a)
{
	instance *player = instance_nearest(a, obj_id_player);
	if (player) {
		cpVect b = player->body->p;
		a = we_cart2pol(a);
		b = we_cart2pol(b);
		cpFloat angle_diff = b.y - a.y;
		return (WE_PI_PI(angle_diff)) * currentlvl->outer_radius;
	}
	return 0;
}

cpVect se_dist_a2b(instance *insa, instance *insb)
{
	if (insa && insb) {
		cpVect a = insa->body->p;
		cpVect b = insb->body->p;
		cpVect d = cpvsub(b,a);
		return d;
	} else {
		return cpv(999999,999999);
	}
}

cpVect se_dist2body(cpBody *b1, cpBody *b2)
{
	if (b1 && b2) {
		return cpvsub(b2->p, b1->p);
	} else {
		return cpv(999999,999999);
	}
}

/**
 * returns 1 if object gets destroyed (returns 0 if already destroyed)
 */
int se_damage_object(instance *object, instance *dmg_dealer)
{
	float *damage = COMPONENT(dmg_dealer, DAMAGE, float*);
	if (damage) {
		return se_damage_deal(object, *damage);
	}
	return 0;
}

int se_damage_deal(instance *object, float damage)
{
	hpbar *hp = COMPONENT(object, HPBAR, hpbar*);
	if (hp && hp->value > 0 && !hp->invinc) {
		hp->value -= damage;
		if (hp->value <= 0) {
			instance_destroy(object);
			return 1;
		}
	}

	return 0;
}

/**
 * returns the best angle to shoot at a moving object obj2 from obj1 (angle between 0 and 2PI?)
 */
cpFloat se_get_best_shoot_angle(cpBody *body1, cpBody *body2, cpFloat bullet_speed)
{
	cpVect p1 = body1->p;//, v1 = body1->v;
	cpVect p2 = body2->p, v2 = body2->v;
	cpVect p21 = cpvsub(p1, p2);
	cpFloat v2_abs = cpvlength(v2);

	cpFloat beta_x = cpvdot(p21, v2) / (cpvlength(v2) * cpvlength(p21));
	cpFloat beta = acos(maxf(minf(beta_x,1),-1));
	cpFloat alpha_y = (v2_abs * sin(beta)) / bullet_speed;
	cpFloat alpha = asin(maxf(minf(alpha_y,1),-1));

	cpFloat p21_angle = cpvtoangle(p21);
	cpFloat beta_y = cpvdot(cpvperp(p21), v2);

	if (beta_y < 0){
		alpha  = -alpha;
	}
	alpha  = WE_PI + (p21_angle - alpha);

	if (alpha != alpha) {
		SDL_Log("ERROR: invalid angle!");
		//raise(SIGKILL);
	}

	return alpha;
}

cpFloat get_angle(cpVect a, cpVect b)
{
	cpFloat bc = cpvtoangle(cpvsub(a,b));
	return bc;
}


//TODO add preferred angle to handle situations with two possible solutions?
float turn_toangle(float from_angle, float to_angle, float step_size) //TODO add limit/constraints
{
	if (from_angle >= WE_2PI) {
		from_angle -= ((int)(from_angle/WE_2PI))*WE_2PI;
	} else if (from_angle < 0) {
		from_angle -= ((int)(from_angle/WE_2PI))*WE_2PI;
	}
	if (to_angle >= WE_2PI) {
		to_angle -= ((int)(to_angle/WE_2PI))*WE_2PI;
	} else if (to_angle < 0) {
		to_angle -= ((int)(to_angle/WE_2PI))*WE_2PI;
	}

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

	return from_angle;
}

void se_spawn_coins(instance * ins)
{
	int *coins_ptr = COMPONENT(ins, COINS, int *);
	if (coins_ptr) {
		cpVect pos = ins->body->p;
		int i = (*coins_ptr) & 0xFFF; /* limit number of coins to 4096*/

		obj_param_coin arg;
		arg.explo_fmax = minf(i*20 + 400, 1500);

		float rnd_x, rnd_y;
		while (i) {
			rnd_x = 2 - ((i & 0x3) + 1);
			rnd_y = 2 - (((i & 0x3) ^ 0x3) + 1);
			instance_create(obj_id_coin, &arg, cpvadd(pos,cpv(rnd_x,rnd_y*2)), cpvzero);
			--i;
		}
	}
}

cpFloat se_tangent(cpVect p)
{
	return cpvtoangle(cpvperp(p));
}

void se_tangent_body(cpBody *body)
{
	cpBodySetAngle(body, se_tangent(body->p));
}

void se_velfunc(cpBody *body, int with_gravity)
{
	switch(with_gravity) {
	case 1:
		body->velocity_func = space_vel_func;
		break;
	case -1:
		body->velocity_func = space_vel_neg_func;
		break;
	case 0:
		body->velocity_func = space_vel_func_zero_g;
		break;
	}
}
