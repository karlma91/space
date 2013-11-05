#include "draw.h"
#include "texture.h"
#include "matrix2d.h"

#include "../engine.h"
#include "../data/stack.h"
#include "../io/waffle_utils.h"

#include "../../space/spaceengine.h" //TODO remove dependency?


GLfloat triangle_quad[8] = {-0.5, -0.5,
		0.5,  -0.5,
		-0.5, 0.5,
		0.5,  0.5};

GLfloat corner_quad[8] = {0, 0,
		1,  0,
		0, 1,
		1,  1};

/* COLOR DEFINITIONS */
const Color COL_WHITE = {255,255,255,255};
const Color COL_BLACK = {  0,  0,  0,255};
const Color COL_RED   = {255,  0,  0,255};
const Color COL_GREEN = {  0,255,  0,255};
const Color COL_BLUE  = {  0,  0,255,255};

GLuint light_buffer, light_texture;

Color rainbow_col[1536];

#define UNIT_CIRCLE_RES 128
static GLfloat unit_circle[UNIT_CIRCLE_RES];
//static void draw_render_light_map();

#define DEBUG SDL_Log( "line: %d\n", __LINE__);

// GL VARIABLE BUFFER
static byte gl_red = 255;
static byte gl_green = 255;
static byte gl_blue = 255;
static byte gl_alpha = 255;
static int gl_blend_src = 0;
static int gl_blend_dst = 0;
static int gl_texture2d = 0;

#if (__MACOSX__ | __WIN32__)
#define glGenFramebuffersOES glGenFramebuffersEXT
#define glBindFramebufferOES glBindFramebufferEXT
#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER_EXT
#define glFramebufferTexture2DOES glFramebufferTexture2DEXT
#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0_EXT
#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE_EXT
#define glCheckFramebufferStatusOES glCheckFramebufferStatusEXT
#define GL_RENDERBUFFER_OES GL_RENDERBUFFER_EXT
#define GL_DEPTH_COMPONENT24_OES GL_DEPTH_COMPONENT24
#define GL_DEPTH_ATTACHMENT_OES GL_DEPTH_ATTACHMENT_EXT
#define glGenRenderbuffersOES glGenRenderbuffersEXT
#define glBindRenderbufferOES glBindRenderbufferEXT
#define glRenderbufferStorageOES glRenderbufferStorageEXT
#define glFramebufferRenderbufferOES glFramebufferRenderbufferEXT
#endif

int draw_init(){

#if LIGHT_SYSTEM
	glGenTextures(1, &light_texture);
	glBindTexture(GL_TEXTURE_2D, light_texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if (__MACOSX__ | __IPHONEOS__)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif

	glGenFramebuffersOES(1,&light_buffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, light_buffer);

	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, light_texture, 0);
#endif

	int i;
	for (i=0; i < UNIT_CIRCLE_RES; i+=2) {
		unit_circle[i] = sinf( WE_2PI*i / (UNIT_CIRCLE_RES-2));
		unit_circle[i+1] = cosf( WE_2PI*i / (UNIT_CIRCLE_RES-2));
	}

	/* generate rainbow colors */
	float min_col = 0.2f;
	Color *c = rainbow_col;
	for(i=0; i <= 255; i++, c++)
		c->r = 255, c->g = i, c->b = min_col, c->a = 255;
	for(i=0; i <= 255; i++, c++)
		c->r = 255 - i, c->g = 255, c->b = min_col, c->a = 255;
	for(i=0; i <= 255; i++, c++)
		c->r = min_col, c->g = 255, c->b = i, c->a = 255;
	for(i=0; i <= 255; i++, c++)
		c->r = min_col, c->g = 255 - i, c->b = 255, c->a = 255;
	for(i=0; i <= 255; i++, c++)
		c->r = i, c->g = min_col, c->b = 255, c->a = 255;
	for(i=0; i <= 255; i++, c++)
		c->r = 255, c->g = min_col, c->b = 255 - i, c->a = 255;
	return 0;
}


void draw_color4b(byte r, byte g, byte b, byte a)
{
	if (r != gl_red || g != gl_green || b != gl_blue || a != gl_alpha) {
		gl_red = r;
		gl_green = g;
		gl_blue = b;
		gl_alpha = a;
	}
}

void draw_color(Color color)
{
	draw_color4b(color.r, color.g, color.b, color.a);
}

void draw_color4f(float r, float g, float b, float a)
{
	draw_color4b((byte)(r*0xFF), (byte)(g*0xFF), (byte)(b*0xFF), (byte)(a*0xFF));
}

void draw_color3f(float r, float g, float b)
{
	draw_color4b((byte)(r*0xFF), (byte)(g*0xFF), (byte)(b*0xFF), gl_alpha);
}

void draw_color_rgbmulta4b(byte r, byte g, byte b, byte a) {
	draw_color4b(r*a/255, g*a/255, b*a/255, a);
}

void draw_color_rgbmulta(Color color)
{
	draw_color_rgbmulta4b(color.r, color.g, color.b, color.a);
}

void draw_color_rgbmulta4f(float r, float g, float b, float a)
{
	draw_color_rgbmulta4b((byte)(r*0xFF), (byte)(g*0xFF), (byte)(b*0xFF), (byte)(a*0xFF));
}


void draw_push_color(void)
{
	stack_push_byte(gl_red);
	stack_push_byte(gl_green);
	stack_push_byte(gl_blue);
	stack_push_byte(gl_alpha);
}

void draw_pop_color(void)
{
	byte a = stack_pop_byte();
	byte b = stack_pop_byte();
	byte g = stack_pop_byte();
	byte r = stack_pop_byte();
	draw_color4b(r,g,b,a);
}

void draw_blend(GLenum src_factor, GLenum dst_factor)
{
	if (gl_blend_src != src_factor || gl_blend_dst != dst_factor) {
		gl_blend_src = src_factor;
		gl_blend_dst = dst_factor;
	}
}

void draw_push_blend(void)
{
	stack_push_int(gl_blend_dst);
	stack_push_int(gl_blend_src);
}

void draw_pop_blend(void)
{
	int src = stack_pop_int();
	int dst = stack_pop_int();
	draw_blend(src, dst);
}

void draw_enable_tex2d(void)
{
	if (!gl_texture2d) {
		gl_texture2d = 1;
		glEnable(GL_TEXTURE_2D);
	}
}
void draw_disable_tex2d(void) // deprecated method!
{
	if (gl_texture2d) {
		gl_texture2d = 0;
		glDisable(GL_TEXTURE_2D);
	}
}

//TODO create stretched sprite polyline render


void draw_line_spr_id(int layer, SPRITE_ID id, cpVect a, cpVect b, float w)
{
	float tex_map[8];
	sprite_get_first_image(id,tex_map);
    int tex_id = sprite_get_texture(id);
	draw_line_tex(layer, tex_id, tex_map, a, b, w);
}

void draw_line_spr(int layer, sprite *spr, cpVect a, cpVect b, float w)
{
	float tex_map[8];
	sprite_get_current_image(spr,tex_map);
    int tex_id = sprite_get_texture(spr->id);
	draw_line_tex(layer, tex_id, tex_map, a, b, w);
}

void draw_line_tex(int layer, int tex_id, float *tex_map, cpVect a, cpVect b, float w)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;

    draw_push_matrix();
    draw_translate(a.x, a.y);
	draw_rotate(atan2f(dy,dx));
	GLfloat length = hypotf(dy, dx);

    w /=2; // tmp-fix
    float x0 = 0;
    float x1 = w;
    float x2 = length - w;
    float x3 = length;

    if (x1 > x2) {
    	x1 = length / 2;
    	x2 = x1;
    }

    float y0 = -w;
    float y1 = w;

    GLfloat line_mesh[16] = {
    		x0, y0, x0, y1,
            x1, y0, x1, y1,
            x2, y0, x2, y1,
            x3, y0, x3, y1};

    //TODO clip inner edges if length < w --> (|||), (||), (|), (), <>, ..
    float tx_1 = tex_map[0];
    float tx_2 = tex_map[2];
    float tx_h = tx_1 + (tx_2-tx_1)/2;
    float ty_1 = tex_map[1];
    float ty_2 = tex_map[5];

    GLfloat line_texture[16] = {tx_1, ty_1,
            tx_1,  ty_2,
            tx_h, ty_1,
            tx_h,  ty_2,
            tx_h, ty_1,
            tx_h,  ty_2,
            tx_2, ty_1,
            tx_2,  ty_2};

    texture_bind_virt(tex_id);
	draw_triangle_strip(layer, line_mesh, line_texture, 8);

    draw_pop_matrix();
}

void draw_glow_line(cpVect a, cpVect b, float w)
{
	draw_push_color();
	Color col = draw_get_current_color();
	col.a = 0;
	draw_color(col);
	draw_line_spr_id(0, SPRITE_GLOW, a, b, w);
	draw_color4b(255,255,255,0);
	draw_line_spr_id(0, SPRITE_DOT, a, b, w);
	draw_pop_color();
}

void draw_quad_line(int layer, cpVect a, cpVect b, float w)
{
	float subimg[8];
	texture_bind_virt(sprite_get_texture(SPRITE_WHITE));
	sprite_get_first_image(SPRITE_WHITE,subimg);
	float dx = b.x-a.x;
	float dy = b.y-a.y;

    draw_push_matrix();
    draw_translate(a.x, a.y);
	draw_rotate(atan2f(dy,dx));
	GLfloat length = hypotf(dy, dx);
	draw_scale(1,w);
	w /= 2;
	GLfloat line[8] = { -w, -0.5,
			-w,  0.5,
			length + w, -0.5,
			length + w,  0.5};

	draw_quad_new(layer, line, subimg);
	draw_pop_matrix();
}

void draw_line_strip(const GLfloat *strip, int l, float w)
{
	int i;
	for(i = 0; i<l-2; i+=2) {
		draw_quad_line(0, cpv(strip[i],strip[i+1]),cpv(strip[i+2],strip[i+3]),w/4);
	}
}

void draw_destroy(void)
{
	//TODO! --> release texture resources
	//...
}

void draw_circle(int layer, cpVect pos, GLfloat radius)
{
	draw_donut(0, pos, 0, radius);
}

void draw_donut(int layer, cpVect p, GLfloat inner_r, GLfloat outer_r)
{
	texture_bind_virt(sprite_get_texture(SPRITE_WHITE));
	int i = 0;
	static float v[UNIT_CIRCLE_RES*2];
	int j = 0;
	for(i = 0;i<UNIT_CIRCLE_RES; i+=2){
		v[j++] = (p.x+unit_circle[i]*inner_r);
		v[j++] = p.y+unit_circle[i+1]*inner_r;
		v[j++] = (p.x+unit_circle[i]*outer_r);
		v[j++] = p.y+unit_circle[i+1]*outer_r;
	}

	i = 0;
	static float t[UNIT_CIRCLE_RES*2];
	sprite_subimg subimg = sprite_get_subimg(SPRITE_WHITE);
	float tx = (subimg.x1 + subimg.x2) / 2;
	float ty = (subimg.y1 + subimg.y2) / 2;
	while (i<UNIT_CIRCLE_RES*2) {
		t[i++] = tx;
		t[i++] = ty;
	}
	draw_triangle_strip(layer, v, t, UNIT_CIRCLE_RES);
}

void draw_box(int layer, cpVect p, cpVect s, GLfloat angle, int centered)
{
	float subimg[8];
	texture_bind_virt(sprite_get_texture(SPRITE_WHITE));
	sprite_get_first_image(SPRITE_WHITE,subimg);
    draw_push_matrix();
    draw_translate(p.x,p.y);
	draw_rotate(angle);
	draw_scale(s.x, s.y);
	draw_quad_new(layer, centered ? triangle_quad : corner_quad, subimg);
	draw_pop_matrix();
}

Color draw_col_rainbow(int hue)
{
	hue = (hue < 0 ? -hue : hue) % 1536;
	return rainbow_col[hue];
}

Color draw_col_grad(int hue)
{
	static int a = 0;
	hue = (((hue < 0 ? -hue : hue)) % (1536/8) + ++a/16) % 1536;
	return rainbow_col[hue];
}

Color draw_get_current_color(void)
{
	Color col = {gl_red, gl_green, gl_blue, gl_alpha};
	return col;
}

Blend draw_get_current_blend(void)
{
	Blend blend = {gl_blend_src, gl_blend_dst};
	return blend;
}

//TODO: color customization
void draw_bar(int layer, cpVect pos, cpVect size, cpFloat angle, cpFloat p, cpFloat p2)
{
	cpVect pos_org = pos;

	/* save current blend function and color */
	draw_push_color();
	draw_push_blend();

	p = (p < 1 ? (p > 0 ? p : 0) : 1);
	p2 = (p2 < 1 ? (p2 > 0 ? p2 : 0) : 1);

	//draw_blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	cpVect rot = cpvforangle(angle);
	/* hp bar */
	if (size.x > size.y) {
		float width = size.y;
		float size_red = size.x * p;
		float size_bar = size.x * p2;
		pos_org = cpvadd(pos_org, cpvmult(rot, -size.x/2));

		cpVect pos_red = cpvadd(pos_org, cpvmult(rot, size_red));
		cpVect pos_bar = cpvadd(pos_org, cpvmult(rot, size_bar));

		draw_color(COL_RED);
		draw_line_spr_id(layer, SPRITE_BAR, pos_org, pos_red, width);

		cpFloat pp = p*p;
		cpFloat pppp = pp*pp;
		cpFloat p_1 = 1-p;

		draw_color4f(1-pppp*pppp, 0.8-p_1*p_1*0.8 + 0.1, 0.1, 1);
		draw_line_spr_id(layer, SPRITE_BAR, pos_org, pos_bar, width);

	} else {
		rot = cpvperp(rot);
		float width = size.x;
		float size_bar = size.y * p;
		pos_org = cpvadd(pos_org, cpvmult(rot, -size.y/2));
		cpVect pos_bar = cpvadd(pos_org, cpvmult(rot, size_bar));
		draw_color4f(1-p,1-p,1,1);
		draw_line_spr_id(layer, SPRITE_BAR, pos_org, pos_bar, width);
	}

	draw_pop_color();
	draw_pop_blend();
}

void draw_polygon_textured(int layer, int count, cpVect *verts, cpVect p, float rotation, float size, float texture_scale, int texture)
{
	float vertices[count* 2];
	float texcoord[count * 2];
	int i, j = 0;
	for (i = 0; i < count; i++) {
		vertices[j] = verts[i].x * 1;
		texcoord[j] = verts[i].x * texture_scale;
		j++;
		vertices[j] = verts[i].y * 1;
		texcoord[j] = verts[i].y * texture_scale;
		j++;
	}

	texture_bind_virt(texture);
	draw_push_matrix();
	draw_translatev(p);
	draw_rotate(rotation);
	draw_scale(size, size);
	draw_color4f(1,1,1,1);
	draw_triangle_fan(layer, vertices, texcoord, count);

	draw_pop_matrix();

}
void draw_polygon_outline(int count, cpVect *verts, cpVect p, float rotation, float size)
{
	draw_push_matrix();
	draw_translatev(p);
	draw_rotate(rotation);
	int i = 0;
	for(i=0; i< count; i++) {
		if(i < count - 1) {
			draw_quad_line(0, cpvmult(verts[i],size), cpvmult(verts[i + 1],size), 4);
		} else {
			draw_quad_line(0, cpvmult(verts[i],size), cpvmult(verts[0],size), 4);
		}
	}
	draw_pop_matrix();
}


void draw_texture(int layer, int tex_id, cpVect pos, const float *tex_map, cpVect size, float angle)
{
	texture_bind_virt(tex_id);
	draw_current_texture_append(layer, pos, tex_map, size, angle);
}

void draw_current_texture_append(int layer, cpVect pos, const float *tex_map, cpVect size, float angle)
{
	draw_push_matrix();
	draw_translate(pos.x, pos.y);
	draw_rotate(angle);
	draw_scalev(size);
	draw_quad_new(layer, triangle_quad, tex_map);
	draw_pop_matrix();
}

void draw_translatev(cpVect offset)
{
    matrix2d_translate(offset.x,offset.y);
}

void draw_translate(GLfloat x, GLfloat y)
{
    matrix2d_translate(x,y);
}

void draw_rotatev(cpVect rot)
{
    matrix2d_rotatev(rot.x, rot.y);
}

void draw_rotate(GLfloat angle)
{
    matrix2d_rotate(angle);
}

void draw_scalev(cpVect scale)
{
    matrix2d_scale(scale.x,scale.y);
}

void draw_scale(GLfloat x, GLfloat y)
{
    matrix2d_scale(x,y);
}

void draw_matrix_clear(void)
{
	matrix2d_clear();
}

void draw_push_matrix(void)
{
    matrix2d_push();
}

void draw_pop_matrix(void)
{
    matrix2d_pop();
}

void draw_load_identity(void)
{
    matrix2d_setidentity();
}
