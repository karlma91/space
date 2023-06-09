#ifndef DRAW_H_
#define DRAW_H_

#include "SDL.h"
#include "we_utils.h"

#define LOAD_TEXTURES 1 //FOR DEBUG PURPOSES
#define EXPERIMENTAL_GRAPHICS 1
#define LIGHT_SYSTEM 0

#if __IPHONEOS__ || __ANDROID__
#define GLES2 1
#else
#define GLES1 0
#define GLES2 0
#endif


#if GLES2
#include "SDL_opengles2.h"
#else

#if __WIN32__
#define GLEW_STATIC
#include "GL/glew.h"
#endif

#define GL_GLEXT_PROTOTYPES 1
#include "GL/glu.h"
#include "GL/glext.h"
#include "SDL_opengl.h"
#endif

#include "chipmunk.h"
#include "texture.h"
#include "sprite.h"
#include "font.h"
#include "bmfont.h"

typedef struct Blend {
	GLenum src_factor;
	GLenum dst_factor;
} Blend;

/* COLOR DECLARATIONS */
extern const Color COL_WHITE;
extern const Color COL_BLACK;
extern const Color COL_RED;
extern const Color COL_GREEN;
extern const Color COL_BLUE;

typedef void (*draw_callback)(int layer, void *drawdata);
typedef struct drawable {
	int datasize;
	draw_callback func;
	void *data;
} drawable;

typedef struct drawbl_dualspr {
	SPRITE_ID spr_id;
	cpVect pos, size;
	Color col1, col2;
	int anti_rotation;
} drawbl_dualspr;
void draw_dualsprite(int layer, drawbl_dualspr *dualspr);

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
void draw_destroy(void);

//void draw_light_map(void);

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

void draw_color_rgbmulta4b(byte r, byte g, byte b, byte a);
void draw_color_rgbmulta4f(float r, float g, float b, float a);
void draw_color_rgbmulta(Color color);

void draw_blend(GLenum src_factor, GLenum dst_factor);

void draw_push_color(void);
void draw_pop_color(void);
void draw_push_blend(void);
void draw_pop_blend(void);

void draw_enable_tex2d(void);
void draw_disable_tex2d(void);

Color draw_get_current_color(void);
Blend draw_get_current_blend(void);

void draw_glow_line(cpVect a, cpVect b, float w);
void draw_line_strip(const GLfloat *strip, int l, float w);
void draw_velocity_line(cpShape *shape);
void draw_quad_line(int layer, cpVect a, cpVect b, float w);
void draw_line_spr_id(int layer, SPRITE_ID id, cpVect a, cpVect b, float w, int append_edge);
void draw_line_spr(int layer, sprite *spr, cpVect a, cpVect b, float w, int append_edge);
void draw_simple_line_spr(int layer, sprite *spr, cpVect a, cpVect b, float w);
void draw_simple_line_spr_id(int layer, SPRITE_ID id, int index, cpVect a, cpVect b, float w);
void draw_line_tex(int layer, int tex_id, float *tex_map, cpVect a, cpVect b, float w, int append_edge);


void draw_box(int layer, cpVect p, cpVect size, GLfloat angle, int centered);
void draw_bar(int layer, cpVect pos, cpVect size, cpFloat angle, cpFloat p, cpFloat p2);
void draw_circle(int layer, cpVect p, GLfloat radius);
void draw_donut(int layer, cpVect p, GLfloat inner_r, GLfloat outer_r);
void draw_polygon_textured(int layer, int count, cpVect *verts, cpVect p, float rotation, float size, float textuer_scale, int texture);
void draw_polygon_outline(int count, cpVect *verts, cpVect p, float rotation, float size);

void draw_texture(int layer, int tex_id, cpVect pos, const float *tex_map, cpVect size, float angle);
void draw_current_texture_append(int layer, cpVect pos, const float *tex_map, cpVect size, float angle);

Color draw_col_rainbow(int hue);
Color draw_col_grad(int hue);

void draw_quad_new(int layer, float ver_quad[8], const float tex_quad[8]); /* appends an independent quad to the current render state */
void draw_quad_continue(int layer, float ver_edge[4], const float tex_edge[4]); /* extends a previous quad by adding 2 vertices specified by edge */
void draw_triangle_strip(int layer, float *ver_list, const float *tex_list, int count); /* appends a triangle strip to current render state */
void draw_triangle_fan(int layer, float *ver_fan, const float *tex_fan, int count); /* converts triangle fan to triangle list and appends to current render state */

void draw_quad_patch_center_tex(int layer, int tex_id, float *tex_map, cpVect pos, cpVect size, float corner, float angle);
void draw_quad_patch_center(int layer, SPRITE_ID id, cpVect pos, cpVect size, float corner, float angle);
void draw_quad_patch_center_spr(int layer, sprite *spr, cpVect pos, cpVect size, float corner, float angle);

#endif /* DRAW_H_ */
