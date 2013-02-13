#ifndef PARTICLES_H_
#define PARTICLES_H_

#include "chipmunk.h"



void particles_init();
void particles_destroy();
void particles_add_explosion(cpVect v, float time, int speed, int numPar, int color);
void particles_draw();
void particles_update();


#endif /* PARTICLES_H_ */
