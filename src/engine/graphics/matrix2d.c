#include <math.h>
#include <stdio.h>

#include "matrix2d.h"
#include "draw.h"
#include <signal.h>

#define STACK_SIZE 1000

static matrix2d stack[STACK_SIZE];
matrix2d *cur = &stack[0];

int current_matrix;

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
	matrix2d_setidentity();
}

void matrix2d_push(void)
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

void matrix2d_pop(void)
{
    if (current_matrix > 0) {
    	--current_matrix;
        --cur;
    } else {
    	SDL_Log("ERROR: matrix2d_popmatrix -> tried to pop without push!");
    }
}

void matrix2d_setidentity(void)
{
    cur->x1 = 1; cur->y1 = 0; cur->z1 = 0;
    cur->x2 = 0; cur->y2 = 1; cur->z2 = 0;
}

void matrix2d_set(matrix2d m)
{
    cur->x1 = m.x1; cur->y1 = m.y1; cur->z1 = m.z1;
    cur->x2 = m.x2; cur->y2 = m.y2; cur->z2 = m.z2;
}

matrix2d matrix2d_get(void)
{
	return *cur;
}

cpVect matrix2d_multcpv(matrix2d m, cpVect v)
{
    return cpv(m.x1*v.x + m.y1*v.y + m.z1, m.x2*v.x + m.y2*v.y + m.z2);
}

void matrix2d_multp(float *point)
{
    float x = point[0];
    float y = point[1];
    point[0] = cur->x1*x + cur->y1*y + cur->z1;
    point[1] = cur->x2*x + cur->y2*y + cur->z2;
}

void matrix2d_multv(float *vertices, int count)
{
	int i;
	for (i = 0; i < count * 2; i += 2) {
		matrix2d_multp(&vertices[i]);
	}
}

void matrix2d_print(void)
{
    fprintf(stderr, "[ %f , %f , %f ]\n",cur->x1,cur->y1,cur->z1);
    fprintf(stderr, "[ %f , %f , %f ]\n",cur->x2,cur->y2,cur->z2);
}

