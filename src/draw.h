//#ifndef SPACE_DRAW_HEADER
//#define SPACE_DRAW_HEADER

#include "chipmunk.h"

#define CIRCLE_SMALL 16
#define CIRCLE_MEDIUM 32
#define CIRCLE_BIG 128
#define CIRCLE_EXTRA 256
#define CIRCLE_MAX_RES 256

// effect texture
unsigned texture[10];


//fra chipmunkDemo.h
typedef struct Color {
	float r, g, b, a;
} Color;

//??? static functions i header filen ???
static inline void glColor_from_color(Color color)
{
	glColor4fv((GLfloat *)&color);
}

static inline Color RGBAColor(float r, float g, float b, float a)
{
	Color color = {r, g, b, a};
	return color;
}

static inline Color LAColor(float l, float a)
{
	Color color = {l, l, l, a};
	return color;
}

Color draw_col_rainbow(int hue);
Color draw_col_grad(int hue);

void draw_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w);
void draw_line_strip(const GLfloat *strip, int l, float w);
void draw_init();
void draw_destroy();
void draw_circle(cpVect center, cpFloat angle, 
								 cpFloat radius, cpFloat scale, 
								 Color fill, Color line);
void draw_polygon(int count, cpVect *verts, Color lineColor, Color fillColor);
void draw_segment(cpVect a, cpVect b, cpFloat width, Color lineColor);
void draw_space(cpSpace *space);
void draw_segmentshape(cpShape *shape);
void draw_boxshape(cpShape *shape);
void draw_ballshape(cpShape *shape);
void draw_shape(cpShape *shape, void *unused);
void draw_simple_box(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
void draw_quad_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w);
