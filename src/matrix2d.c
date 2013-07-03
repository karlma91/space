#include <math.h>
#include "matrix2d.h"
#include <stdio.h>
#include "draw.h"
float stack[50][6];

float *cur = stack[0];

float *vertex_pointer;
float *tex_pointer;

float vertex[30000];
float tex[30000];
float color[30000];

int current;

int blend_function[2];
int prev_blend_function[2];

static void multiply_current(float *f);

int * matrix2d_get_blend_func()
{
    return blend_function;
}

void matrix2d_tex_pointer(float *f)
{
    tex_pointer = f;
}

float * matrix2d_get_tex_pointer()
{
    return tex;
}

float * matrix2d_get_current_tex_pointer()
{
    return tex_pointer;
}

void matrix2d_vertex_pointer(float *f)
{
    vertex_pointer = f;
}

float *matrix2d_get_vertex_pointer()
{
    return vertex;
}

float *matrix2d_get_color_pointer()
{
    return color;
}

float * matrix2d_get_current_vertex_pointer()
{
    return vertex_pointer;
}

void matrix2d_reset()
{

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
    current += 1;
    int i;
    for (i=0; i<6; i++){
        stack[current][i] = stack[current-1][i];
    }
    cur = stack[current];
}

void matrix2d_popmatrix()
{
    if(current > 0){
        current -= 1;
        cur = stack[current];
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
