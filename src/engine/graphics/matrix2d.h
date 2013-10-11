
#ifndef MATRIX2D_H_
#define MATRIX2D_H_

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

float *matrix2d_get_color_data(void);
float *matrix2d_get_color_pointer(void);
void matrix2d_color_pointer(float *f);

void matrix2d_translate(float x, float y);
void matrix2d_rotatev(float c, float s);
void matrix2d_rotate(float a);
void matrix2d_scale(float x, float y);
void matrix2d_setmatrix(float *m);

void matrix2d_clear(void);
void matrix2d_pushmatrix(void);
void matrix2d_loadindentity(void);
void matrix2d_popmatrix(void);

void matrix2d_multiply_current(int count);
void matrix2d_multiply_point(float *point);
void matrix2d_multiply(float *vertices, int count);


void matrix2d_append_strip(int first, int count);
void matrix2d_append_quad_tex(void);
void matrix2d_append_quad_color(void);
void matrix2d_append_quad_tex_color(void);
void matrix2d_append_quad_simple(void);

int matrix2d_get_count(void);

void matrix2d_print(void);

#endif /* MATRIX2D_H_ */
