
#ifndef MATRIX2D_H_
#define MATRIX2D_H_

void matrix2d_reset();

float * matrix2d_get_tex_data();
float * matrix2d_get_tex_pointer();
void matrix2d_tex_pointer(float *f);

float * matrix2d_get_vertex_data();
float * matrix2d_get_vertex_pointer();
void matrix2d_vertex_pointer(float *f);

float *matrix2d_get_color_data();
float *matrix2d_get_color_pointer();
void matrix2d_color_pointer(float *f);

int * matrix2d_get_blend_func();

void matrix2d_translate(float x, float y);
void matrix2d_rotate(float a);
void matrix2d_scale(float x, float y);
void matrix2d_setmatrix(float *m);

void matrix2d_pushmatrix();
void matrix2d_loadindentity();
void matrix2d_popmatrix();

void matrix2d_multiply_current(int count);
void matrix2d_multiply_point(float *point);
void matrix2d_multiply(float *vertices, int count);


void matrix2d_append_strip(int first, int count);
void matrix2d_append_quad_tex();
void matrix2d_append_quad_color();
void matrix2d_append_quad_tex_color();
void matrix2d_append_quad_simple();

int matrix2d_get_count();

void matrix2d_print();

#endif /* MATRIX2D_H_ */
