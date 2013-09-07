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

void se_add_score_and_popup(cpVect p, int score)
{
    particles_add_score_popup(p, score);
	((obj_player *) instance_first(obj_id_player))->coins += score;
}

float se_distance_to_player(float x)
{
	obj_player *player = ((obj_player *) instance_first(obj_id_player));
	if(player){
		cpVect a = cpv(x,0);
		cpVect b = player->data.body->p;
		cpVect d = se_distance_v(a, b);
		return d.x;
	}
	return 0;
}

cpVect se_distance_a2b(instance *insa, instance *insb)
{
	if (insa && insb) {
		cpVect a = insa->body->p;
		cpVect b = insb->body->p;
		cpVect d = se_distance_v(a,b);
		return d;
	} else {
		return cpv(999999,999999);
	}
}

cpVect se_distance_v(cpVect a, cpVect b)
{
	//FIXME (not working properly at level edge?)
	cpVect d = cpvsub(b,a);
	int lvl_width = currentlvl->width;
	if(d.x < -lvl_width/2){
		d.x += lvl_width;
	}else if(d.x > lvl_width/2){
		d.x -= lvl_width;
	}else{
		fabsf(d.x);
	}
	return d;
}


/**
 * return 1 if object is killed
 */
int se_damage_object(instance *object, instance *dmg_dealer)
{
	float *damage = COMPONENT(dmg_dealer, DAMAGE, float*);
	if (damage) {
		hpbar *hp = COMPONENT(object, HPBAR, hpbar*);
		hp->value -= *damage;
		if (hp->value <= 0) {
			instance_destroy(object);
			return 1;
		}
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

	cpFloat g = cpvdot(v, vb)/(cpvlength(vb) * cpvlength(v));
	cpFloat G = acos(g < 1 ? (g > -1 ? g : -1) : 1);
	cpFloat angle = asin((c * sin(G)) / s);

	cpFloat bc = cpvtoangle(v);

	if(vb.x < 0){
		angle  = -angle;
	}
	angle  = M_PI + (bc - angle);

	if (angle != angle) {
		SDL_Log("ERROR: invalid angle!");
		raise(SIGKILL);
	}

	return angle;
}

cpFloat get_angle(cpVect a, cpVect b)
{
	cpFloat bc = cpvtoangle(cpvsub(a,b));
	return bc;
}


//TODO add preferred angle to handle situations with two possible solutions
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

float se_rect2arch(cpVect *pos)
{
#if EXPERIMENTAL_GRAPHICS
	if (!space_rendering_map || (statesystem_get_render_state() != state_space))
		return 0;

	float inner_radius = currentlvl->inner_radius;
	float theta = -currentlvl->theta_max * (current_camera->x - pos->x) / ((current_camera->right - current_camera->left)/2);

	float o_x = current_camera->x;
	float o_y = currentlvl->height + inner_radius;

	float ry = currentlvl->height - pos->y;

	pos->x = o_x + (inner_radius + ry) * sinf(theta);
	pos->y = o_y - (inner_radius + ry) * cosf(theta);

	return theta;
#else
	return 0;
#endif
}

void se_rect2arch_column(float x, cpVect *polar)
{
#if EXPERIMENTAL_GRAPHICS
	float theta = -currentlvl->theta_max *(current_camera->x - x) / ((current_camera->right - current_camera->left)/2);

    polar->x = sinf(theta);
    polar->y = cosf(theta);
#endif
}

void se_rect2arch_from_data(cpVect *pos, cpVect *polar)
{
#if EXPERIMENTAL_GRAPHICS
	float inner_radius = currentlvl->inner_radius;
    float o_x = current_camera->x;
    float o_y = currentlvl->height + inner_radius;

    float ry = currentlvl->height - pos->y;

    pos->x = o_x + (inner_radius + ry) * polar->x;
    pos->y = o_y - (inner_radius + ry) * polar->y;
#endif
}


//TODO check bounding box vs camera view
//TODO move inside a camera class
int se_inside_view(cpVect *pos, float margin)
{
	int dx = current_camera->x - (int) pos->x;
	int lvl_width = currentlvl->width;
	int cam_distance = (dx < -lvl_width/2) ? abs(dx + lvl_width) : (dx > lvl_width/2) ? dx - lvl_width : abs(dx);

	return cam_distance <= current_camera->width + margin;
}

void se_spawn_coins(instance * ins)
{
	int *coins_ptr = COMPONENT(ins, COINS, int *);
	if (coins_ptr) {
		cpVect pos = ins->body->p;
		int i = (*coins_ptr) & 0xFFF; /* limit number of coins to 4096*/
		float rnd_x, rnd_y;
		while (i) {
			rnd_x = 2 - ((i & 0x3) + 1);
			rnd_y = 2 - (((i & 0x3) ^ 0x3) + 1);
			instance_create(obj_id_coin, NULL, pos.x + rnd_x, pos.y + rnd_y*2, 0, 0);
			--i;
		}
	}
}

