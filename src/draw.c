#include "draw.h"
#include "texture.h"
#include "stack.h"
#include "waffle_utils.h"
#include "spaceengine.h"

GLfloat triangle_quad[8] = {-0.5, -0.5,
						 0.5,  -0.5,
						  -0.5, 0.5,
						  0.5,  0.5};

GLfloat corner_quad[8] = {0, 0,
							 1,  0,
							  0, 1,
							  1,  1};

Color rainbow_col[1536];

static GLfloat unit_circle[128];

#define DEBUG SDL_Log( "line: %d\n", __LINE__);

GLfloat color_stack[10];


int draw_init(){

	int i=0,j=0;
	for(i = 0; i < 128; i += 2) {
		unit_circle[i] = sinf( 2*M_PI*i / (128-2));
		unit_circle[i+1] = cosf( 2*M_PI*i / (128-2));
	}

	/* Photoshop Outer glow settings:
	 * Opacity 50%
	 * Spread 60%
	 * Size 32px
	 * Range 100%
	 * Jitter 0%
	 */
	/*
	 *
	 */

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

void draw_color(Color color)
{
#if GLES1
	draw_color4f(color.r, color.g, color.b, color.a);
#else
	glColor4fv((GLfloat *)&color);
#endif
}

void draw_push_color()
{
	GLfloat color[4];
	glGetFloatv(GL_CURRENT_COLOR, color);
	stack_push_float(color[0]);
	stack_push_float(color[1]);
	stack_push_float(color[2]);
	stack_push_float(color[3]);
}

void draw_pop_color()
{
	GLfloat a = stack_pop_float();
	GLfloat b = stack_pop_float();
	GLfloat g = stack_pop_float();
	GLfloat r = stack_pop_float();
	draw_color4f(r,g,b,a);
}

void draw_push_blend()
{
	int dst;
	int src;
	glGetIntegerv(GL_BLEND_DST, &dst);
	glGetIntegerv(GL_BLEND_SRC, &src);
	stack_push_int(dst);
	stack_push_int(src);
}

void draw_pop_blend()
{
	int src = stack_pop_int();
	int dst = stack_pop_int();
	glBlendFunc(src, dst);
}

void draw_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w)
{

	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	draw_push_blend();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glTranslatef(x0, y0, 0.0f);
	glRotatef(atan2f(y1-y0,x1-x0)*(180/M_PI), 0.0f, 0.0f, 1.0f);
	GLfloat length = sqrtf((y1-y0)*(y1-y0) + (x1-x0)*(x1-x0));
	glScalef(1,w,1);

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

	glVertexPointer(2, GL_FLOAT, 0, line_mesh);
	glTexCoordPointer( 2, GL_FLOAT, 0, line_texture );

	texture_bind(TEX_GLOW);
	glDrawArrays(GL_TRIANGLE_STRIP,0, 8);

	draw_color4f(1,1,1,1);
	texture_bind(TEX_DOT);
	glDrawArrays(GL_TRIANGLE_STRIP,0, 8);

	draw_pop_blend();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

}

void draw_quad_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w)
{
	glPushMatrix();
	glTranslatef(x0, y0, 0.0f);
	glRotatef(atan2f(y1-y0,x1-x0)*(180/M_PI), 0.0f, 0.0f, 1.0f);
	GLfloat length = sqrtf((y1-y0)*(y1-y0) + (x1-x0)*(x1-x0));
	glScalef(1,w,1);
	w /= 2;
	GLfloat line[8] = { -w, -0.5,
			-w,  0.5,
			length + w, -0.5,
			length + w,  0.5};

	glVertexPointer(2, GL_FLOAT, 0, line);
	glDrawArrays(GL_TRIANGLE_STRIP,0, 4);

	glPopMatrix();
}

void draw_line_strip(const GLfloat *strip, int l, float w)
{
	int i;
	for(i = 0; i<l-2; i+=2) {
		draw_quad_line(strip[i],strip[i+1],strip[i+2],strip[i+3],w/4);
	}
}

void draw_color4f(float r, float g, float b, float a)
{
	glColor4f(r,g,b,a);
}

void draw_destroy()
{

	//TODO! --> release texture resources
	//...
}

void draw_circle(GLfloat x, GLfloat y, GLfloat radius)
{
#if GLES1


#else
	int i;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0,0);
	for(i = 0;i<128; i+=2){
		glVertex2f(unit_circle[i]*radius, unit_circle[i+1]*radius);
	}
	glEnd();
#endif
}

void draw_donut(GLfloat x, GLfloat y, GLfloat inner_r, GLfloat outer_r)
{
#if GLES1


#else
	int i;
	glBegin(GL_TRIANGLE_STRIP);
	for(i = 0;i<128; i+=2){
		glVertex2f(unit_circle[i]*inner_r, unit_circle[i+1]*inner_r);
		glVertex2f(unit_circle[i]*outer_r, unit_circle[i+1]*outer_r);
	}
	glEnd();
#endif
}

void draw_box(GLfloat x, GLfloat y, GLfloat w, GLfloat h,GLfloat angle,int centered)
{
	glPushMatrix();
	glTranslatef(x,y,0);
	glRotatef(angle,0,0,1);
	glScalef(w,h,1);
	glVertexPointer(2, GL_FLOAT, 0, centered ? triangle_quad : corner_quad);
	glDrawArrays(GL_TRIANGLE_STRIP,0, 4);
	glPopMatrix();
}


void draw_simple_circle(GLfloat x, GLfloat y, GLfloat radius,GLfloat rot)
{
#if GLES1


#else
	int i;
	glPushMatrix();
	glTranslatef(x,y,0);
	glRotatef(rot,0,0,1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0,0);
	for(i = 0;i<128; i+=2){
		glVertex2f(unit_circle[i]*radius, unit_circle[i+1]*radius);
	}
	glEnd();
	draw_color4f(1,1,1,1);
	draw_box(0, 0, radius, 5, 0, 1);
	glPopMatrix();
#endif
}

void draw_polygon(int count, cpVect *verts, Color lineColor, Color fillColor)
{
#if GLES1


#else
#if CP_USE_DOUBLES
	glVertexPointer(2, GL_DOUBLE, 0, verts);
#else
	glVertexPointer(2, GL_FLOAT, 0, verts);
#endif

	if(fillColor.a > 0){
		draw_color(fillColor);
		glDrawArrays(GL_TRIANGLE_FAN, 0, count);
	}

	if(lineColor.a > 0){
		draw_color(lineColor);
		glDrawArrays(GL_LINE_LOOP, 0, count);
	}
#endif
}

void draw_segment(cpVect a, cpVect b, cpFloat w, Color lineColor)
{
#if GLES1


#else
	draw_color(lineColor);
	glBegin(GL_QUADS);
	glVertex2f(a.x, a.y - w);
	glVertex2f(a.x, a.y + w);
	glVertex2f(b.x, b.y + w);
	glVertex2f(b.x, b.y - w);
	glEnd();
	//draw_line(a.x, a.y, b.x, b.y, 10);
#endif
}

void draw_ballshape(cpShape *shape)
{
	cpCircleShape *circle = (cpCircleShape *)shape;
	cpBody *body = shape->body;
	//cpVect vel = cpBodyGetVel(cpShapeGetBody(shape));
	draw_simple_circle(circle->tc.x, circle->tc.y, circle->r, cpBodyGetAngle(body)*(180/M_PI)); //40 = 4 * radius
}
void draw_velocity_line(cpShape *shape)
{
	cpCircleShape *circle = (cpCircleShape *)shape;
	cpVect vel = cpBodyGetVel(cpShapeGetBody(shape));
	draw_line(circle->tc.x, circle->tc.y, circle->tc.x - vel.x/64, circle->tc.y - vel.y/64, 32); //40 = 4 * radius
}

void draw_boxshape(cpShape *shape, Color a, Color b)
{
	glLineWidth(2);
	cpPolyShape *poly = (cpPolyShape *)shape;
	draw_polygon(poly->numVerts, poly->tVerts,a,b);
}

void draw_segmentshape(cpShape *shape)
{
	cpSegmentShape *seg = (cpSegmentShape *)shape;
	draw_segment(seg->ta, seg->tb, seg->r, RGBAColor(0.80f, 0.107f, 0.05f,1.0f));
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

//TODO: color customization
void draw_bar(cpFloat x, cpFloat y, cpFloat w, cpFloat h, cpFloat p, cpFloat p2)
{
	cpVect pos = {x, y};
	float angle = 0;

#if EXPERIMENTAL_GRAPHICS
	angle = se_rect2arch(&pos) * 180 / M_PI;
#endif

	x = pos.x;
	y = pos.y;

	float border;
	/* save current blend function and color */
	draw_push_color();
	draw_push_blend();

	p = (p < 1 ? (p > 0 ? p : 0) : 1);
	p2 = (p2 < 1 ? (p2 > 0 ? p2 : 0) : 1);

	/* outer edge */
	draw_color4f(1, 1, 1, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_box(x, y, w, h, angle, 0);

	/* inner edge */

	border = 0.1 * (w > h ? h : w);
	draw_color4f(0, 0, 0, 1);
	draw_box(x + border, y + border, w - border * 2, h - border * 2, angle, 0);

	/* hp bar */
	border *= 2;
	if (w > h) {
		draw_color4f(1,0,0, 1);
		draw_box(x + border, y + border, (w - border * 2) * p, h - border * 2, angle, 0);
		draw_color4f(1-((p*p)*(p*p))*((p*p)*(p*p)), 0.8-(1-p)*(1-p)*0.8 + 0.1, 0.1, 1);
		draw_box(x + border, y + border, (w - border * 2) * p2, h - border * 2, angle, 0);
	} else {
		draw_color4f(1-p,1-p,1,1);
		draw_box(x + border, y + border, w - border * 2, (h - border * 2) * p, angle, 0);
	}

	draw_pop_color();
	draw_pop_blend();
}

void draw_texture(int tex_id, cpVect *pos, const float *tex_map, float width, float height, float angle)
{
	cpVect pos_buf = *pos;
#if EXPERIMENTAL_GRAPHICS
	angle += se_rect2arch(&pos_buf) * 180 / M_PI; //EXPERIMENTAL GRAPHICS
#endif

	texture_bind(tex_id);
	draw_current_texture_all(&pos_buf, tex_map, width, height, angle, triangle_quad);
}

void draw_current_texture(cpVect *pos, const float *tex_map, float width, float height, float angle)
{
	draw_current_texture_all(pos, tex_map, width, height, angle, triangle_quad);
}

void draw_current_texture_all(cpVect *pos, const float *tex_map, float width, float height, float angle, GLfloat *mesh)
{
	glPushMatrix();
	glTranslatef(pos->x, pos->y, 0.0f);
	glRotatef(angle,0,0,1);
	glScalef(width,height,1);

	draw_current_texture_basic(tex_map, mesh, 4);

	glPopMatrix();
}

void draw_current_texture_basic(const float *tex_map, GLfloat *mesh, GLsizei count)
{
	glVertexPointer(2, GL_FLOAT, 0, mesh);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_map);

	glEnable(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLE_STRIP,0, count);
	glDisable(GL_TEXTURE_2D);
}

void draw_translate(GLfloat x, GLfloat y, GLfloat z)
{
	glTranslatef(x,y,z);
}

void draw_rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	glRotatef(angle,x,y,z);
}

void draw_scale(GLfloat x, GLfloat y, GLfloat z)
{
	glScalef(x,y,z);
}

void draw_push_matrix()
{
	glPushMatrix();
}

void draw_pop_matrix()
{
	glPopMatrix();
}

void draw_load_identity()
{
	glLoadIdentity();
}

// todo combine different gl pointers into an interleaved array
float *draw_append_quad(float *data, float *mesh)
{
	// {A, B, C, D} -> {A, B, C, B, C, D}
	*data++ = mesh[0]; //A.x
	*data++ = mesh[1]; //A.y
	*data++ = mesh[2]; //B.x
	*data++ = mesh[3]; //B.y
	*data++ = mesh[4]; //C.x
	*data++ = mesh[5]; //C.y
	*data++ = mesh[4]; //C.x
	*data++ = mesh[5]; //C.y
	*data++ = mesh[2]; //B.x
	*data++ = mesh[3]; //B.y
	*data++ = mesh[6]; //D.x
	*data++ = mesh[7]; //D.y

	return data;
}

void draw_flush(float *vertex, float *uv, int size)
{
	glVertexPointer(2, GL_FLOAT, 0, vertex);
	glTexCoordPointer(2, GL_FLOAT, 0, uv);

	glDrawArrays(GL_TRIANGLES, 0, size / 2);
}

