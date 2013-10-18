#ifndef DRAW_H_
#define DRAW_H_

#include "SDL.h"
#include "we_utils.h"

#define LOAD_TEXTURES 1 //FOR DEBUG PURPOSES
#define EXPERIMENTAL_GRAPHICS 1
#define LIGHT_SYSTEM 0

#if __IPHONEOS__ || __ANDROID__
#define GLES1 1
#else
#define GLES1 0
#endif



#if GLES1
//#include "SDL_opengles.h"
#include "SDL_opengles2.h"
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


/* COLOR DECLARATIONS */
extern const Color COL_WHITE;
extern const Color COL_BLACK;
extern const Color COL_RED;
extern const Color COL_GREEN;
extern const Color COL_BLUE;

static inline Color RGBAColor4b(byte r, byte g, byte b, byte a)
{
	Color color = {r, g, b, a};
	return color;
}

static inline Color RGBAColor4f(float r, float g, float b, float a)
{
	return RGBAColor4b((byte)(r*0xff),(byte)(g*0xff),(byte)(b*0xff),(byte)(a*0xff));
}

static inline Color LAColor2b(byte l, byte a)
{
	return RGBAColor4b(l,l,l,a);
}

static inline Color LAColor2f(float l, float a)
{
	return LAColor2b((byte)(l*0xff),(byte)(a*0xff));
}

extern GLfloat triangle_quad[8];
extern GLfloat corner_quad[8];

int draw_init(void);
void draw_light_map(void);

void draw_translatev(cpVect offset);
void draw_translate(GLfloat x, GLfloat y);
void draw_rotatev(cpVect rot);
void draw_rotate(GLfloat angle);
void draw_scalev(cpVect scale);
void draw_scale(GLfloat x, GLfloat y);

void draw_matrix_clear(void);
void draw_push_matrix(void);
void draw_pop_matrix(void);
void draw_load_identity(void);

void draw_color4b(byte r, byte g, byte b, byte a);
void draw_color4f(float r, float g, float b, float a);
void draw_color3f(float r, float g, float b);
void draw_color(Color color);
void draw_blend(GLenum src_factor, GLenum dst_factor);

void draw_push_color(void);
void draw_pop_color(void);
void draw_push_blend(void);
void draw_pop_blend(void);

void draw_enable_tex2d(void);
void draw_disable_tex2d(void);
void draw_push_tex2d(void);
void draw_pop_tex2d(void);

void draw_draw_arrays(GLenum mode, GLint first, GLsizei count);
void draw_vertex_pointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void draw_tex_pointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void draw_get_current_color(float *c);

void draw_line(int tex_id, cpVect a, cpVect b, float w);
void draw_glow_line(cpVect a, cpVect b, float w);
void draw_line_strip(const GLfloat *strip, int l, float w);
void draw_destroy(void);
void draw_circle(cpVect p, GLfloat radius);
void draw_donut(cpVect p, GLfloat inner_r, GLfloat outer_r);
void draw_polygon_textured(int count, cpVect *verts, cpVect p, float rotation, float size, float textuer_scale, int texture);
void draw_polygon_outline(int count, cpVect *verts, cpVect p, float rotation, float size);
void draw_segment(cpVect a, cpVect b, cpFloat width, Color lineColor);
void draw_segmentshape(cpShape *shape);
void draw_boxshape(cpShape *shape, Color a, Color b);
void draw_velocity_line(cpShape *shape);
void draw_shape(cpShape *shape, void *unused);
void draw_box(cpVect p, cpVect size, GLfloat angle, int centered);
void draw_quad_line(cpVect a, cpVect b, float w);
void draw_bar(cpVect pos, cpVect size, cpFloat angle, cpFloat p, cpFloat p2);

void draw_texture(int tex_id, cpVect pos, const float *tex_map, cpVect size, float angle);
void draw_current_texture_basic(const float *tex_map, GLfloat *mesh, GLsizei count);
void draw_current_texture(cpVect pos, const float *tex_map, cpVect size, float angle);
void draw_current_texture_all(cpVect pos, const float *tex_map, cpVect size, float angle, GLfloat *mesh);

void draw_current_texture_append(cpVect pos, const float *tex_map, cpVect size, float angle);
void draw_box_append(cpVect p, cpVect s, GLfloat angle, int centered);

void draw_sprite_line(sprite *spr, cpVect a, cpVect b, float w);

void draw_append_quad(void);
void draw_append_quad_simple(void);
void draw_append_color_quad(void);
void draw_append_color_tex_quad(void);
void draw_flush(void);
void draw_flush_and_multiply(void);
void draw_flush_color(void);
void draw_flush_simple(void);

Color draw_col_rainbow(int hue);
Color draw_col_grad(int hue);

#endif /* DRAW_H_ */
