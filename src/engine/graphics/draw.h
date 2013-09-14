#ifndef DRAW_H_
#define DRAW_H_

#include "SDL.h"

#define LOAD_TEXTURES 1 //FOR DEBUG PURPOSES
#define EXPERIMENTAL_GRAPHICS 0
#define LIGHT_SYSTEM 0

#if __IPHONEOS__ || __ANDROID__
#define GLES1 1
#else
#define GLES1 0
#endif



#if GLES1
#include "SDL_opengles.h"
#else

#if __WIN32__
#define GLEW_STATIC
#include "GL/glew.h"
#endif

#include "SDL_opengl.h"
#endif

#include "chipmunk.h"
#include "texture.h"
#include "sprite.h"
#include "font.h"
#include "bmfont.h"

//fra chipmunkDemo.h
typedef struct Color {
	float r, g, b, a;
} Color;

/* COLOR DECLARATIONS */
extern const Color COL_WHITE;
extern const Color COL_BLACK;
extern const Color COL_RED;
extern const Color COL_GREEN;
extern const Color COL_BLUE;

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

extern GLfloat triangle_quad[8];
extern GLfloat corner_quad[8];

void draw_light_map(void);

void draw_push_color(void);
void draw_pop_color(void);
void draw_push_blend(void);
void draw_pop_blend(void);

void draw_translate(GLfloat x, GLfloat y, GLfloat z);
void draw_rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void draw_scale(GLfloat x, GLfloat y, GLfloat z);
void draw_push_matrix(void);
void draw_pop_matrix(void);
void draw_load_identity(void);
void draw_draw_arrays(GLenum mode, GLint first, GLsizei count);
void draw_vertex_pointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void draw_tex_pointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void draw_get_current_color(float *c);

Color draw_col_rainbow(int hue);
Color draw_col_grad(int hue);
void draw_color4f(float r, float g, float b, float a);
void draw_color(Color color);
void draw_line(int tex_id, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w);
void draw_glow_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w);
void draw_line_strip(const GLfloat *strip, int l, float w);
int draw_init(void);
void draw_destroy(void);
void draw_circle(cpVect p, GLfloat radius);
void draw_donut(cpVect p, GLfloat inner_r, GLfloat outer_r);
void draw_polygon(int count, cpVect *verts, Color lineColor, Color fillColor);
void draw_segment(cpVect a, cpVect b, cpFloat width, Color lineColor);
void draw_space(cpSpace *space);
void draw_segmentshape(cpShape *shape);
void draw_boxshape(cpShape *shape, Color a, Color b);
void draw_velocity_line(cpShape *shape);
void draw_shape(cpShape *shape, void *unused);
void draw_box(GLfloat x, GLfloat y, GLfloat w, GLfloat h,GLfloat angle,int centered);
void draw_simple_circle(GLfloat x, GLfloat y, GLfloat radius,GLfloat rot);
void draw_quad_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w);
void draw_bar(cpFloat x, cpFloat y, cpFloat w, cpFloat h, cpFloat p, cpFloat p2);
extern void draw_texture(int tex_id, cpVect *pos, const float *tex_map, float width, float height, float angle);
void draw_current_texture_basic(const float *tex_map, GLfloat *mesh, GLsizei count);
void draw_current_texture(cpVect *pos, const float *tex_map, float width, float height, float angle);
void draw_current_texture_all(cpVect *pos, const float *tex_map, float width, float height, float angle, GLfloat *mesh);

void draw_current_texture_append(cpVect *pos, const float *tex_map, float width, float height, float angle);
void draw_box_append(GLfloat x, GLfloat y, GLfloat w, GLfloat h,GLfloat angle,int centered);

void draw_sprite_line(sprite *spr, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w);


void draw_append_quad(void);
void draw_append_quad_simple(void);
void draw_append_color_quad(void);
void draw_append_color_tex_quad(void);
void draw_flush(void);
void draw_flush_and_multiply(void);
void draw_flush_color(void);
void draw_flush_simple(void);

#endif /* DRAW_H_ */
