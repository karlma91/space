
#ifndef MATRIX2D_H_
#define MATRIX2D_H_

#include "chipmunk.h"
#include "we_utils.h"

typedef struct {
    float x1, y1, z1;
    float x2, y2, z2;
} matrix2d;

void matrix2d_reset(void);

float * matrix2d_get_tex_data(void);
float * matrix2d_get_tex_pointer(void);
void matrix2d_tex_pointer(float *f);

float * matrix2d_get_vertex_data(void);
float * matrix2d_get_vertex_pointer(void);
void matrix2d_vertex_pointer(float *f);

int matrix2d_get_stride(void);
int matrix2d_get_type(void);
void matrix2d_set_stride(int stride);
void matrix2d_set_type(int type);

byte *matrix2d_get_color_data(void);
byte *matrix2d_get_color_pointer(void);
void matrix2d_color_pointer(byte *f);

void matrix2d_translate(float x, float y);
void matrix2d_rotatev(float c, float s);
void matrix2d_rotate(float a);
void matrix2d_scale(float x, float y);
void matrix2d_setmatrix(float *m);
matrix2d matrix2d_getmatrix();

void matrix2d_clear(void);
void matrix2d_pushmatrix(void);
void matrix2d_loadindentity(void);
void matrix2d_popmatrix(void);

void matrix2d_multiply_current(int count);
void matrix2d_multiply_point(float *point);
void matrix2d_multiply(float *vertices, int count);
cpVect matrix2d_transform_vect(matrix2d m, cpVect v);

void matrix2d_append_strip(int first, int count);
void matrix2d_append_quad_tex(void);
//void matrix2d_append_quad_color(void);
void matrix2d_append_quad_tex_color(void);
void matrix2d_append_quad_simple(void);

int matrix2d_get_count(void);

void matrix2d_print(void);

#endif /* MATRIX2D_H_ */
