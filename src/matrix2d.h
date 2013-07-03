
#ifndef MATRIX2D_H_
#define MATRIX2D_H_

void matrix2d_reset();
void matrix2d_vertex_pointer(float *f);
void matrix2d_tex_pointer(float *f);
float * matrix2d_get_current_tex_pointer();
float * matrix2d_get_tex_pointer();
float * matrix2d_get_vertex_pointer();
float * matrix2d_get_current_vertex_pointer();

float *matrix2d_get_color_pointer();
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
void matrix2d_append(int first, int count);

void matrix2d_print();

#endif /* MATRIX2D_H_ */
