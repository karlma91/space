#include <math.h>
#include "matrix2d.h"
#include <stdio.h>
#include "draw.h"
float stack[50][6];

float *cur = stack[0];

float *vertex_pointer;
float *tex_pointer;
float *color_pointer;

float vertex[30000];
float tex[30000];
float color[30000];

float * vertex_append = vertex;
float * tex_append = tex;
float * color_append = color;


int current_matrix;

int blend_function[2];
int prev_blend_function[2];

static void multiply_current(float *f);
static float *matrix2d_append_quad(float *data, float *mesh);
static void append_quad_color();
static float * matrix2d_multiply_to_quad(float *data, float *mesh, int count);

int * matrix2d_get_blend_func()
{
    return blend_function;
}

/*
 * Texture
 */
float * matrix2d_get_tex_data()
{
    return tex;
}

float * matrix2d_get_tex_pointer()
{
    return tex_pointer;
}

void matrix2d_tex_pointer(float *f)
{
    tex_pointer = f;
}

/*
 * Vertex
 */
float *matrix2d_get_vertex_data()
{
    return vertex;
}

float * matrix2d_get_vertex_pointer()
{
    return vertex_pointer;
}

void matrix2d_vertex_pointer(float *f)
{
    vertex_pointer = f;
}

/*
 * Color
 */
float *matrix2d_get_color_data()
{
    return color;
}

float * matrix2d_get_color_pointer()
{
    return color_pointer;
}

void matrix2d_color_pointer(float *f)
{
	color_pointer = f;
}

int matrix2d_get_count()
{
	return (vertex_append - vertex);
}

static void append_quad_color()
{
	float c[4];
	draw_get_current_color(c);
	int i,j;
	for(i=0; i<6; i++){
		for(j=0; j<4;j++){
			*color_append++ = c[j];
		}
	}
}


void matrix2d_append_strip(int first, int count)
{

}

void matrix2d_append_quad_simple()
{
	vertex_append = matrix2d_multiply_to_quad(vertex_append, vertex_pointer, 4);
}

void matrix2d_append_quad_tex()
{
	tex_append = matrix2d_append_quad(tex_append, tex_pointer);
	vertex_append = matrix2d_multiply_to_quad(vertex_append, vertex_pointer,4);
}

void matrix2d_append_quad_color()
{
	append_quad_color();
	vertex_append = matrix2d_multiply_to_quad(vertex_append, vertex_pointer,4);
}
void matrix2d_append_quad_tex_color()
{
	append_quad_color();
	tex_append = matrix2d_append_quad(tex_append, tex_pointer);
	vertex_append = matrix2d_multiply_to_quad(vertex_append,vertex_pointer, 4);
}

static float *matrix2d_append_quad(float *data, float *mesh)
{
	// {A, B, C, D} -> {A, B, C, B, C, D}
	*data++ = mesh[0]; //A.x
	*data++ = mesh[1]; //A.y
	*data++ = mesh[0]; //A.x
	*data++ = mesh[1]; //A.y
	*data++ = mesh[2]; //B.x
	*data++ = mesh[3]; //B.y
	*data++ = mesh[4]; //C.x
	*data++ = mesh[5]; //C.y
	*data++ = mesh[6]; //D.x
	*data++ = mesh[7]; //D.y
	*data++ = mesh[6]; //D.x
	*data++ = mesh[7]; //D.y

	return data;
}


void matrix2d_reset()
{
	vertex_append = vertex;
	tex_append = tex;
	color_append = color;
}

void matrix2d_translate(float x, float y)
{
    cur[2] = cur[0]*x + cur[1]*y + cur[2];
    cur[5] = cur[3]*x + cur[4]*y + cur[5];
}

void matrix2d_rotate(float a)
{
    float c = cosf(a);
    float s = sinf(a);

    float c0 = cur[0];
    cur[0] = c0*c + cur[1]*s;
    cur[1] = -c0*s + cur[1]*c;

    float c3 = cur[3];
    cur[3] = c3*c + cur[4]*s;
    cur[4] = -c3*s + cur[4]*c;
}

void matrix2d_scale(float x, float y)
{
    cur[0] = cur[0]*x;
    cur[1] = cur[1]*y;
    cur[3] = cur[3]*x;
    cur[4] = cur[4]*y;
}

void matrix2d_pushmatrix()
{
    current_matrix += 1;
    int i;
    for (i=0; i<6; i++){
        stack[current_matrix][i] = stack[current_matrix-1][i];
    }
    cur = stack[current_matrix];
}

void matrix2d_popmatrix()
{
    if(current_matrix > 0){
        current_matrix -= 1;
        cur = stack[current_matrix];
    }
}

void matrix2d_loadindentity()
{
    cur[0] = 1;
    cur[1] = 0;
    cur[2] = 0;

    cur[3] = 0;
    cur[4] = 1;
    cur[5] = 0;
}

void matrix2d_setmatrix(float *m)
{
    int i;
    for (i=0; i<6; i++){
        cur[i] = m[i];
    }
}

void matrix2d_multiply_current(int count)
{
    int i;
    for(i=0; i<count*2; i++){
        vertex[i] = vertex_pointer[i];
    }
    matrix2d_multiply(vertex,count);
}

void matrix2d_multiply(float *vertices, int count)
{
    int i;
    for(i=0; i<count*2; i+=2){
        matrix2d_multiply_point(&vertices[i]);
    }
}
static float * matrix2d_multiply_to_quad(float *data, float *mesh, int count)
{

	float x = mesh[0];
	float y = mesh[1];
	*data++ = cur[0]*x + cur[1]*y + cur[2];
	*data++ = cur[3]*x + cur[4]*y + cur[5];
    int i;
    for(i=0; i<count*2; i+=2){
    	 x = mesh[i];
    	 y = mesh[i+1];
    	*data++ = cur[0]*x + cur[1]*y + cur[2];
    	*data++ = cur[3]*x + cur[4]*y + cur[5];
    }
    x = mesh[count*2-2];
    y = mesh[count*2-1];
    *data++ = cur[0]*x + cur[1]*y + cur[2];
    *data++ = cur[3]*x + cur[4]*y + cur[5];
    return data;
}


void matrix2d_multiply_point(float *point)
{
    float x = point[0];
    float y = point[1];
    point[0] = cur[0]*x + cur[1]*y + cur[2];
    point[1] = cur[3]*x + cur[4]*y + cur[5];
}

void matrix2d_print()
{
    fprintf(stderr, "[ %f , %f , %f ]\n",cur[0],cur[1],cur[2]);
    fprintf(stderr, "[ %f , %f , %f ]\n",cur[3],cur[4],cur[5]);
}

static void multiply_current(float *f)
{
    float c0 = cur[0];
    float c1 = cur[1];
    cur[0] = c0*f[0] + c1*f[3];
    cur[1] = c0*f[1] + c1*f[4];
    cur[2] = c0*f[2] + c1*f[5] + cur[2];

    float c3 = cur[3];
    float c4 = cur[4];
    cur[3] = c3*f[0] + c4*f[3];
    cur[4] = c3*f[1] + c4*f[4];
    cur[5] = c3*f[2] + c4*f[5] + cur[5];
}
