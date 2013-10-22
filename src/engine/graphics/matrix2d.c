#include <math.h>
#include <stdio.h>

#include "matrix2d.h"
#include "draw.h"
#include <signal.h>

#define STACK_SIZE 1000

static matrix2d stack[STACK_SIZE];
matrix2d *cur = &stack[0];

int stride;
int type;

float *vertex_pointer;
float *tex_pointer;
byte *color_pointer;

float vertex[300000];
float tex[300000];
byte color[300000];

float * vertex_append = vertex;
float * tex_append = tex;
byte * color_append = color;


int current_matrix;

//static void multiply_current(float *f);
static float *matrix2d_append_quad(float *data, float *mesh);
static void append_quad_color();
static float * matrix2d_multiply_to_quad(float *data, float *mesh, int count);

/*
 * Texture
 */
float * matrix2d_get_tex_data(void)
{
    return tex;
}

float * matrix2d_get_tex_pointer(void)
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
float *matrix2d_get_vertex_data(void)
{
    return vertex;
}

float * matrix2d_get_vertex_pointer(void)
{
    return vertex_pointer;
}

void matrix2d_vertex_pointer(float *f)
{
    vertex_pointer = f;
}

int matrix2d_get_stride(void)
{
    return stride;
}

int matrix2d_get_type(void)
{
    return type;
}

void matrix2d_set_stride(int stri)
{
	stride = stri;
}
void matrix2d_set_type(int typ)
{
	type = typ;
}

/*
 * Color
 */
byte *matrix2d_get_color_data(void)
{
    return color;
}

byte *matrix2d_get_color_pointer(void)
{
    return color_pointer;
}

void matrix2d_color_pointer(byte *f)
{
	color_pointer = f;
}

int matrix2d_get_count(void)
{
	return (vertex_append - vertex);
}

//TODO append color for only one vertex per quad (of 6 vertices) using stride = 6
static void append_quad_color(void)
{
	byte c[4];
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

void matrix2d_append_quad_simple(void)
{
	append_quad_color();
	vertex_append = matrix2d_multiply_to_quad(vertex_append, vertex_pointer, 4);
}

void matrix2d_append_quad_tex(void)
{
	append_quad_color();
	tex_append = matrix2d_append_quad(tex_append, tex_pointer);
	vertex_append = matrix2d_multiply_to_quad(vertex_append, vertex_pointer,4);
}

//void matrix2d_append_quad_color(void)
//{
//	append_quad_color();
//	vertex_append = matrix2d_multiply_to_quad(vertex_append, vertex_pointer,4);
//}
void matrix2d_append_quad_tex_color(void)
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


void matrix2d_reset(void)
{
	vertex_append = vertex;
	tex_append = tex;
	color_append = color;
}

void matrix2d_translate(float x, float y)
{
    cur->z1 += cur->x1*x + cur->y1*y;
    cur->z2 += cur->x2*x + cur->y2*y;
}

void matrix2d_rotatev(float c, float s)
{
	float x,y;

    x = cur->x1;
    y = cur->y1;
    cur->x1 = x*c + y*s;
    cur->y1 = y*c - x*s;

    x = cur->x2;
    y = cur->y2;
    cur->x2 = x*c + y*s;
    cur->y2 = y*c - x*s;
}
void matrix2d_rotate(float a)
{
	matrix2d_rotatev(cosf(a),sinf(a));
}

void matrix2d_scale(float x, float y)
{
    cur->x1 *= x;
    cur->y1 *= y;
    cur->x2 *= x;
    cur->y2 *= y;
}

void matrix2d_clear(void)
{
	current_matrix = 0;
	cur = &stack[0];
	matrix2d_loadindentity();
}

void matrix2d_pushmatrix(void)
{
	if (current_matrix < STACK_SIZE - 1) {
		++current_matrix;
		matrix2d *m = cur+1;

		m->x1 = cur->x1; m->y1 = cur->y1; m->z1 = cur->z1;
		m->x2 = cur->x2; m->y2 = cur->y2; m->z2 = cur->z2;

		cur = m;
	} else {
		SDL_Log("ERROR: matrix out of bounds!");
		raise(SIGSEGV);
		exit(1);
	}
}

void matrix2d_popmatrix(void)
{
    if (current_matrix > 0) {
    	--current_matrix;
        --cur;
    } else {
    	SDL_Log("ERROR: matrix2d_popmatrix -> tried to pop without push!");
    }
}

void matrix2d_loadindentity(void)
{
    cur->x1 = 1; cur->y1 = 0; cur->z1 = 0;
    cur->x2 = 0; cur->y2 = 1; cur->z2 = 0;
}

void matrix2d_setmatrix(float *m)
{
    cur->x1 = m[0]; cur->y1 = m[1]; cur->z1 = m[2];
    cur->x2 = m[3]; cur->y2 = m[4]; cur->z2 = m[5];
}

matrix2d matrix2d_getmatrix()
{
	return *cur;
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

cpVect matrix2d_transform_vect(matrix2d m, cpVect v)
{
    return cpv(m.x1*v.x + m.y1*v.y + m.z1, m.x2*v.x + m.y2*v.y + m.z2);
}

static float * matrix2d_multiply_to_quad(float *data, float *mesh, int count)
{

	float x = mesh[0];
	float y = mesh[1];
	*data++ = cur->x1*x + cur->y1*y + cur->z1;
	*data++ = cur->x2*x + cur->y2*y + cur->z2;
    int i;
    for(i=0; i<count*2; i+=2){
    	 x = mesh[i];
    	 y = mesh[i+1];
    	*data++ = cur->x1*x + cur->y1*y + cur->z1;
    	*data++ = cur->x2*x + cur->y2*y + cur->z2;
    }
    x = mesh[count*2-2];
    y = mesh[count*2-1];
    *data++ = cur->x1*x + cur->y1*y + cur->z1;
    *data++ = cur->x2*x + cur->y2*y + cur->z2;
    return data;
}


void matrix2d_multiply_point(float *point)
{
    float x = point[0];
    float y = point[1];
    point[0] = cur->x1*x + cur->y1*y + cur->z1;
    point[1] = cur->x2*x + cur->y2*y + cur->z2;
}

void matrix2d_print(void)
{
    fprintf(stderr, "[ %f , %f , %f ]\n",cur->x1,cur->y1,cur->z1);
    fprintf(stderr, "[ %f , %f , %f ]\n",cur->x2,cur->y2,cur->z2);
}

/*
static void multiply_current(float *f)
{
    float c0 = cur->x1;
    float c1 = cur->y1;
    cur->x1 = c0*f[0] + c1*f[3];
    cur->y1 = c0*f[1] + c1*f[4];
    cur->z1 += c0*f[2] + c1*f[5];

    float c3 = cur->x2;
    float c4 = cur->y2;
    cur->x2 = c3*f[0] + c4*f[3];
    cur->y2 = c3*f[1] + c4*f[4];
    cur->z2 += c3*f[2] + c4*f[5];
}
*/

