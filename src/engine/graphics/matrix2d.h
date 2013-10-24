
#ifndef MATRIX2D_H_
#define MATRIX2D_H_

#include "chipmunk.h"
#include "we_utils.h"

typedef struct {
    float x1, y1, z1;
    float x2, y2, z2;
} matrix2d;

void matrix2d_translate(float x, float y);
void matrix2d_rotatev(float c, float s);
void matrix2d_rotate(float a);
void matrix2d_scale(float x, float y);
void matrix2d_set(matrix2d m);
matrix2d matrix2d_get(void);

void matrix2d_clear(void);
void matrix2d_push(void);
void matrix2d_setidentity(void);
void matrix2d_pop(void);

void matrix2d_mult2f(float *point);
void matrix2d_multv(float *vertices, int count);
cpVect matrix2d_multcpv(matrix2d m, cpVect v);

void matrix2d_print(void);

#endif /* MATRIX2D_H_ */
