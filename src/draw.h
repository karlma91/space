#ifndef DRAW_H_
#define DRAW_H_

#include "SDL.h"

#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#elif __ANDROID__
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include "SDL_opengl.h"

#endif

#if TARGET_OS_IPHONE || __ANDROID__
#define GLES1 1
#define GLES1 1
#endif

#if GLES1
	#define glColor3f(r,g,b) glColor4f(r,g,b,1)
#elif GLES1
	#define glColor3f draw_color3f
	#define glColor4f draw_color4f
#endif

#include "chipmunk.h"
#include "texture.h"

//fra chipmunkDemo.h
typedef struct Color {
	float r, g, b, a;
} Color;

//??? static functions i header filen ???
static inline void glColor_from_color(Color color)
{
#if GLES1


#else
	glColor4fv((GLfloat *)&color);
#endif
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
void draw_color3f(float r, float g, float b);
void draw_color4f(float r, float g, float b, float a);
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
void draw_bar(cpFloat x, cpFloat y, cpFloat w, cpFloat h, cpFloat p, cpFloat p2);
extern void draw_texture(int tex_id, cpVect *pos, const texture_map *tex_map, float width, float height, float angle);
void draw_current_texture(cpVect *pos, const texture_map *tex_map, float width, float height, float angle);


#endif /* DRAW_H_ */
