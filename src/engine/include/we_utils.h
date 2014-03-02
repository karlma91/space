/*
 * we_utils.h
 *
 *  Created on: Sep 14, 2013
 *      Author: mathiahw
 */

#ifndef WE_UTILS_H_
#define WE_UTILS_H_
#include "chipmunk.h"
#include "SDL_log.h"
#include <signal.h>
#include <stdio.h>

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

#define WE_INSIDE_RECT(px, py, x1, y1, x2, y2) ((px >= x1) && (px <= x2) && (py >= y1) && (py <= y2))
#define WE_INSIDE_RECT_DIM(px, py, x1, y1, w, h) ((px >= x1) && (px <= x1+w) && (py >= y1) && (py <= y1+h))

typedef unsigned int we_bool;
#define WE_TRUE 1
#define WE_FALSE 0

typedef unsigned char byte;
typedef struct Color {
	byte r, g, b, a;
} Color;

Color color_new3b(byte r, byte g, byte b);
Color color_new4b(byte r, byte g, byte b, byte a);
Color color_new3f(float r, float g, float b);
Color color_new4f(float r, float g, float b, float a);

static inline void we_error(char * msg)
{
	SDL_Log(msg);
	raise(SIGSEGV);
}

void strtolower(char *to, const char *from);

#endif /* WE_UTILS_H_ */
