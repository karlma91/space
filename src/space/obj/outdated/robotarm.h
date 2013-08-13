#if ROBOT_ARM_OBJECT

#ifndef ROBOTARM_H_
#define ROBOTARM_H_

#include "objects.h"

extern obj_preset type_robotarm;

struct robotarm;
struct robotarm_param;

extern instance *robotarm_init(float xpos, struct robotarm_param *param);

struct robotarm {
	/*standard*/
	instance obj;

	/*tank*/
	cpShape *shape;
	float timer;
	float hp;
	int segments;
	float seg_length;
	float *x;
	float *y;
	float *angle;

	/*parameters */
	struct robotarm_param *param;
};

struct robotarm_param {
	float max_hp;
};

#endif /* ROBOTARM_H_ */

#endif
