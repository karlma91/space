/*
 * we_utils.h
 *
 *  Created on: Sep 14, 2013
 *      Author: mathiahw
 */

#ifndef WE_UTILS_H_
#define WE_UTILS_H_

#include "chipmunk.h"

#ifndef WE_DEBUG
#define WE_DEBUG 0
#endif

#define WE_PI_6 0.52359877559
#define WE_PI_4 0.78539816339
#define WE_PI_3 1.0471975512
#define WE_PI_2 1.57079632679
#define WE_PI 3.14159265359
#define WE_3PI_2 4.71238898038
#define WE_2PI 6.28318530718
#define WE_180_PI 57.2957795131

#define we_rad2deg(x) ((x) * WE_180_PI)
#define we_deg2rad(x) ((x) / WE_180_PI)

#define we_randf ((float)rand() / RAND_MAX)

#define minf(a,b) (a <= b ? a : b)
#define maxf(a,b) (a >= b ? a : b)

#define WE_PI_PI(a) (a > WE_PI || a < -WE_PI ? a - roundf(a / WE_2PI) * WE_2PI : a)

#define WE_P2C(r, a) cpvmult(cpvforangle(a),r)
#define we_pol2cart(vect) WE_P2C(vect.x,vect.y)
#define we_cart2pol(cart) cpv(hypotf(cart.x,cart.y),atan2f(cart.y,cart.x))

#endif /* WE_UTILS_H_ */
