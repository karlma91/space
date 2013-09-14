/*
 * we_utils.h
 *
 *  Created on: Sep 14, 2013
 *      Author: mathiahw
 */

#ifndef WE_UTILS_H_
#define WE_UTILS_H_

#include "chipmunk.h"

#define WE_PI 3.14159265359
#define WE_2PI 6.28318530718
#define WE_PI_2 1.57079632679
#define WE_180_PI 57.2957795131

#define we_rad2deg(x) ((x) * WE_180_PI)
#define we_deg2rad(x) ((x) / WE_180_PI)

#define we_randf ((float)rand() / RAND_MAX)

#define minf(a,b) (a <= b ? a : b)
#define maxf(a,b) (a >= b ? a : b)

#define WE_P2C(r, a) cpvmult(cpvforangle(a),r)
#define we_pol2cart(vect) vect=WE_P2C(vect.x,vect.y)
#define we_cart2pol(vect) \
	{float x=vect.x,y=vect.y; vect.x=hypotf(x,y),vect.y=atan2f(y,x);}

#endif /* WE_UTILS_H_ */
