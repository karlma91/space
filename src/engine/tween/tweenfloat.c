#include "tween.h"


tween * tween_float_is_done_remove(tween *t, float *a )
{
	if(t != NULL){
		if(t->d.done) {
			tween_release(t);
			return NULL;
		}else {
		    tween_step(t, dt);
			tween_pos(t, a);
		}
	}
	return t;
}
