#ifndef DRAW_H_
#define DRAW_H_

#include "SDL.h"
#include "SDL_opengl.h"
#include "chipmunk.h"
#include "texture.h"

#define CIRCLE_SMALL 16
#define CIRCLE_MEDIUM 32
#define CIRCLE_BIG 128
#define CIRCLE_EXTRA 256
#define CIRCLE_MAX_RES 256

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
int draw_init();
void draw_destroy();
void draw_circle(GLfloat x, GLfloat y, GLfloat radius);
void draw_donut(GLfloat x, GLfloat y, GLfloat inner_r, GLfloat outer_r);
void draw_polygon(int count, cpVect *verts, Color lineColor, Color fillColor);
void draw_segment(cpVect a, cpVect b, cpFloat width, Color lineColor);
void draw_space(cpSpace *space);
void draw_segmentshape(cpShape *shape);
void draw_boxshape(cpShape *shape, Color a, Color b);
void draw_velocity_line(cpShape *shape);
void draw_shape(cpShape *shape, void *unused);
void draw_simple_box(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
void draw_simple_circle(GLfloat x, GLfloat y, GLfloat radius,GLfloat rot);
void draw_quad_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w);
void draw_hp(cpFloat x, cpFloat y, cpFloat w, cpFloat h, cpFloat p);
extern void draw_texture(int tex_id, cpVect *pos, const texture_map *tex_map, float width, float height, float angle);


#endif /* DRAW_H_ */
