#include "tween.h"


static void cpv_to_array(float * f, cpVect a)
{
	f[0] = a.x;
	f[1] = a.y;
}

tween * tween_cpv_create(cpVect start, cpVect end, float duration, float (*easing)(float))
{
	float s[2];
	cpv_to_array(s, start);
	float e[2];
	cpv_to_array(e, end);
	tween *t = tween_create(s, e, 2, duration, easing);
	return t;
}

/**
 * TODO: use array tween function
 */
tween * tween_cpv_is_done_remove(tween *t, cpVect *a )
{
	if(t != NULL){
		if(t->d.done) {
			tween_release(t);
			return NULL;
		}else {
		    tween_step(t, dt);
			*a = tween_cpv_pos(t);
		}
	}
	return t;
}
cpVect tween_cpv_pos(tween *t)
{
	cpVect v;
	v.x = tween_pos_i(t,0);
	v.y = tween_pos_i(t,1);
	return v;
}
