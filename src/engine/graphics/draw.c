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

static int TEX_BAR;

static GLfloat unit_circle[128];
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

	int i=0,j=0;
	for(i = 0; i < 128; i += 2) {
		unit_circle[i] = sinf( 2*M_PI*i / (128-2));
		unit_circle[i+1] = cosf( 2*M_PI*i / (128-2));
	}

	TEX_BAR = texture_load("bar.png");

	/* generate rainbow colors */
	float min_col = 0.2f;
	Color *c = &rainbow_col[j];
	for(i=0; i <= 255; i++, c++)
		c->r = 1, c->g = i/255.0f, c->b = min_col, c->a = 1;
	for(i=0; i <= 255; i++, c++)
		c->r = 1 - i/255.0f, c->g = 1, c->b = min_col, c->a = 1;
	for(i=0; i <= 255; i++, c++)
		c->r = min_col, c->g = 1, c->b = i/255.0f, c->a = 1;
	for(i=0; i <= 255; i++, c++)
		c->r = min_col, c->g = 1 - i/255.0f, c->b = 1, c->a = 1;
	for(i=0; i <= 255; i++, c++)
		c->r = i/255.0f, c->g = min_col, c->b = 1, c->a = 1;
	for(i=0; i <= 255; i++, c++)
		c->r = 1, c->g = min_col, c->b = 1 - i/255.0f, c->a = 1;
	return 0;
}


void draw_color4b(byte r, byte g, byte b, byte a)
{
	if (r != gl_red || g != gl_green || b != gl_blue || a != gl_alpha) {
		//glColor4ub(r,g,b,a); //TODO There should be no need to call this anymore
		gl_red = r;
		gl_green = g;
		gl_blue = b;
		gl_alpha = a;
	}
}

void draw_color(Color color)
{
#if GLES1
	draw_color4b(color.r, color.g, color.b, color.a);
#else
	draw_color4b(color.r, color.g, color.b, color.a);
	//glColor4ubv((GLubyte *)&color);
#endif
}

void draw_color4f(float r, float g, float b, float a)
{
	draw_color4b((byte)(r*0xFF), (byte)(g*0xFF), (byte)(b*0xFF), (byte)(a*0xFF));
}

void draw_color3f(float r, float g, float b)
{
	draw_color4b((byte)(r*0xFF), (byte)(g*0xFF), (byte)(b*0xFF), gl_alpha);
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
		//draw_flush(); //TODO There should be no need to call this anymore
		//glBlendFunc(src_factor, dst_factor); //TODO There should be no need to call this anymore
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

void draw_line(int tex_id, cpVect a, cpVect b, float w)
{
	float dx = b.x-a.x;
	float dy = b.y-a.y;

	draw_push_matrix();
	draw_push_blend();
	draw_translate(a.x, a.y);
	draw_rotate(atan2f(dy,dx));

	GLfloat length = hypotf(dx,dy);
	draw_scale(1,w);

	w /=2; // tmp-fix

	GLfloat line_mesh[16] = {-w, -0.5,
			-w,  0.5,
			0, -0.5,
			0,  0.5,
			length, -0.5,
			length,  0.5,
			length+w, -0.5,
			length+w,  0.5};

	GLfloat line_texture[16] = {0, 0,
			0,  1,
			0.5, 0,
			0.5,  1,
			0.5, 0,
			0.5,  1,
			1, 0,
			1,  1};

	texture_bind_virt(tex_id);
	draw_triangle_fan(TMP_RENDER_LAYER, line_mesh, line_texture, 8);

	draw_pop_blend();
	draw_pop_matrix();
}

void draw_sprite_line(sprite *spr, cpVect a, cpVect b, float w)
{
//#warning Very similair to draw_line!!
	float dx = b.x-a.x;
	float dy = b.y-a.y;

    draw_push_matrix();
    draw_push_matrix();

    draw_translate(a.x, a.y);
	draw_rotate(atan2f(dy,dx));
	GLfloat length = hypotf(dy, dx);
    draw_scale(1,w);

    w /=2; // tmp-fix

    GLfloat line_mesh[16] = {-w, -0.5,
            -w,  0.5,
            0, -0.5,
            0,  0.5,
            length, -0.5,
            length,  0.5,
            length+w, -0.5,
            length+w,  0.5};

    GLfloat tex_map[8];
    sprite_get_current_image(spr,tex_map);

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

    texture_bind_virt(sprite_get_texture(spr));
	draw_triangle_fan(TMP_RENDER_LAYER, line_mesh, line_texture, 8);

    draw_pop_matrix();
}

//TODO be able to set blend layers
void draw_glow_line(cpVect a, cpVect b, float w)
{
	draw_line(TEX_GLOW, a, b, w);
	draw_color3f(1,1,1);
	draw_line(TEX_GLOW_DOT, a, b, w);
}

void draw_quad_line(cpVect a, cpVect b, float w)
{
	texture_bind_virt(TEX_WHITE);
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

	draw_quad_new(TMP_RENDER_LAYER, line, TEX_MAP_FULL);
	draw_pop_matrix();

//#warning Very similair to draw_line...
}

void draw_line_strip(const GLfloat *strip, int l, float w)
{
	int i;
	for(i = 0; i<l-2; i+=2) {
		draw_quad_line(cpv(strip[i],strip[i+1]),cpv(strip[i+2],strip[i+3]),w/4);
	}
}

void draw_destroy(void)
{
	//TODO! --> release texture resources
	//...
}

void draw_circle(cpVect pos, GLfloat radius)
{
	draw_donut(pos, 0, radius);
}

void draw_donut(cpVect p, GLfloat inner_r, GLfloat outer_r)
{
	texture_bind_virt(TEX_WHITE);
	int i = 0;
	static float v[256];
	int j = 0;
	for(i = 0;i<128; i+=2){
		v[j++] = (p.x+unit_circle[i]*inner_r);
		v[j++] = p.y+unit_circle[i+1]*inner_r;
		v[j++] = (p.x+unit_circle[i]*outer_r);
		v[j++] = p.y+unit_circle[i+1]*outer_r;
	}
	draw_triangle_strip(TMP_RENDER_LAYER, v, TEX_MAP_FULL, 128);
}

void draw_box(cpVect p, cpVect s, GLfloat angle, int centered)
{
	texture_bind_virt(TEX_WHITE);
    draw_push_matrix();
    draw_translate(p.x,p.y);
	draw_rotate(angle);
	draw_scale(s.x, s.y);
	draw_quad_new(TMP_RENDER_LAYER, centered ? triangle_quad : corner_quad, TEX_MAP_FULL);
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
void draw_bar(cpVect pos, cpVect size, cpFloat angle, cpFloat p, cpFloat p2)
{
	cpVect pos_org = pos;

	/* save current blend function and color */
	draw_push_color();
	draw_push_blend();

	p = (p < 1 ? (p > 0 ? p : 0) : 1);
	p2 = (p2 < 1 ? (p2 > 0 ? p2 : 0) : 1);

	//draw_blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* hp bar */
	if (size.x > size.y) {
		cpVect size_red = {size.x * p, size.y};
		cpVect size_bar = {size.x * p2, size.y};

		cpVect pos_red = pos_org;//cpvadd(pos_org, cpv(border + width_red/2, border + height/2));
		cpVect pos_bar = pos_org;//cpvadd(pos_org, cpv(border + width_bar/2, border + height/2));

		draw_color4f(1,0,0, 1);
		//draw_box(x + border, y + border, width_red, height, angle, 0);
		draw_texture(TEX_BAR, pos_red, TEX_MAP_FULL, size_red, angle);

		cpFloat pp = p*p;
		cpFloat pppp = pp*pp;
		cpFloat p_1 = 1-p;

		draw_color4f(1-pppp*pppp, 0.8-p_1*p_1*0.8 + 0.1, 0.1, 1);
		//draw_box(x + border, y + border, width_bar, height, angle, 0);
		draw_texture(TEX_BAR, pos_bar, TEX_MAP_FULL, size_bar, angle);

	} else {
		cpVect size_bar = {size.y * p, size.x};
		cpVect pos_bar = pos_org;//cpvadd(pos_org, cpvmult(size, 0.5));
		draw_color4f(1-p,1-p,1,1);
		draw_texture(TEX_BAR, pos_bar, TEX_MAP_FULL, size_bar, angle + M_PI_2);
	}

	draw_pop_color();
	draw_pop_blend();
}

void draw_polygon_textured(int count, cpVect *verts, cpVect p, float rotation, float size, float textuer_scale, int texture)
{
	float vertices[count* 2];
	float texcoord[count * 2];
	int i, j = 0;
	for (i = 0; i < count; i++) {
		vertices[j] = verts[i].x * 1;
		texcoord[j] = verts[i].x * textuer_scale;
		j++;
		vertices[j] = verts[i].y * 1;
		texcoord[j] = verts[i].y * textuer_scale;
		j++;
	}

	texture_bind_virt(texture);
	draw_push_matrix();
	draw_translatev(p);
	draw_rotate(rotation);
	draw_scale(size, size);
	draw_color4f(1,1,1,1);
	draw_triangle_fan(TMP_RENDER_LAYER+1, vertices, texcoord, count);

	draw_pop_matrix();

}
void draw_polygon_outline(int count, cpVect *verts, cpVect p, float rotation, float size)
{
	texture_bind_virt(TEX_WHITE);
	draw_push_matrix();
	draw_translatev(p);
	draw_rotate(rotation);
	int i = 0;
	for(i=0; i< count; i++) {
		if(i < count - 1) {
			draw_quad_line(cpvmult(verts[i],size), cpvmult(verts[i + 1],size), 1);
		} else {
			draw_quad_line(cpvmult(verts[i],size), cpvmult(verts[0],size), 1);
		}
	}
	draw_pop_matrix();
}


void draw_texture(int tex_id, cpVect pos, const float *tex_map, cpVect size, float angle)
{
	texture_bind_virt(tex_id);
	draw_current_texture_append(pos, tex_map, size, angle);
}

void draw_current_texture_append(cpVect pos, const float *tex_map, cpVect size, float angle)
{
	draw_push_matrix();
	draw_translate(pos.x, pos.y);
	draw_rotate(angle);
	draw_scalev(size);
	draw_quad_new(TMP_RENDER_LAYER, triangle_quad, tex_map);
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
    glLoadIdentity();
    matrix2d_setidentity();
}
