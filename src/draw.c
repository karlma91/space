#include "draw.h"

//local function
static int loadTexture(char *tex);

GLfloat array[CIRCLE_MAX_RES];

static int i, j;
unsigned texture[10];

extern unsigned int *textures; //TMP

static int texC = 0;

Color rainbow_col[1536];

static GLfloat unit_circle[128];

#define DEBUG fprintf(stderr, "line: %d\n", __LINE__);

//TODO TMP remove:
unsigned int loadePNGTexture(char *file);

int draw_init(){

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
	int error;
	error = loadTexture("textures/glowdot.bmp");
	if(error){
		fprintf(stderr, "could not load glowdot.bmp");
		return error;
	}
	error = loadTexture("textures/dot.bmp");
	if(error){
		fprintf(stderr, "could not load dot.bmp");
		return error;
	}
	*/
	//TODO remove TMP
	 loadePNGTexture("textures/glowdot.png"); loadePNGTexture("textures/glow.png"); texture[0] = textures[0]; texture[1] = textures[1];

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

static int loadTexture(char *tex)
{
	SDL_Surface *surface;
	if((surface = SDL_LoadBMP(tex))==NULL){
		fprintf(stderr,"Unable to loade texture\n");
		return 1;
	}
	glGenTextures(1,&texture[texC]);
	
	//bind texture object
	glBindTexture(GL_TEXTURE_2D,texture[texC]);
	// set texture's stretching properties
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	surface->format->Amask = 0XFF000000;
	surface->format->Ashift = 24;
	glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0,
		     GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
	
	SDL_FreeSurface(surface);
	texC++;
	return 0;
}



void draw_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w)
{
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glTranslatef(x0, y0, 0.0f);
		glRotatef(atan2(y1-y0,x1-x0)*(180/M_PI), 0.0f, 0.0f, 1.0f);
		GLfloat length = sqrt((y1-y0)*(y1-y0) + (x1-x0)*(x1-x0));
		glScalef(1,w,1);
	
		w /=2; // tmp-fix
	
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glBegin(GL_QUAD_STRIP);
			glTexCoord2d(0, 0); glVertex2d(-w, -0.5f);
			glTexCoord2d(0, 1); glVertex2d(-w, 0.5f);
			glTexCoord2d(0.5f, 0); glVertex2d(0, -0.5f);
			glTexCoord2d(0.5f, 1); glVertex2d(0, 0.5f);
			glTexCoord2d(0.5f, 0); glVertex2d(length, -0.5f);
			glTexCoord2d(0.5f, 1); glVertex2d(length, 0.5f);
			glTexCoord2d(1.0f, 0); glVertex2d(length+w, -0.5f);
			glTexCoord2d(1.0f, 1); glVertex2d(length+w, 0.5f);
		glEnd();

		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glBegin(GL_QUAD_STRIP);
			glTexCoord2d(0, 0); glVertex2d(-w, -0.5f);
			glTexCoord2d(0, 1); glVertex2d(-w, 0.5f);
			glTexCoord2d(0.5f, 0); glVertex2d(0, -0.5f);
			glTexCoord2d(0.5f, 1); glVertex2d(0, 0.5f);
			glTexCoord2d(0.5f, 0); glVertex2d(length, -0.5f);
			glTexCoord2d(0.5f, 1); glVertex2d(length, 0.5f);
			glTexCoord2d(1.0f, 0); glVertex2d(length+w, -0.5f);
			glTexCoord2d(1.0f, 1); glVertex2d(length+w, 0.5f);
		glEnd();

	glPopAttrib();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}
void draw_quad_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w)
{
	glPushMatrix();
		glTranslatef(x0, y0, 0.0f);
		glRotatef(atan2(y1-y0,x1-x0)*(180/M_PI), 0.0f, 0.0f, 1.0f);
		GLfloat length = sqrt((y1-y0)*(y1-y0) + (x1-x0)*(x1-x0));
		glScalef(1,w,1);
		w /= 2;
		glBegin(GL_QUAD_STRIP);
			glVertex2d(-w, -0.5f);
			glVertex2d(-w, 0.5f);
			glVertex2d(length + w, -0.5f);
			glVertex2d(length + w, 0.5f);
		glEnd();
	glPopMatrix();
}

void draw_line_strip(const GLfloat *strip, int l, float w)
{
	for(i = 0; i<l-2; i+=2) {
		draw_quad_line(strip[i],strip[i+1],strip[i+2],strip[i+3],w/4);
	}
}


void draw_destroy()
{
	
	//TODO! --> release texture resources
	//...
}

void draw_circle(GLfloat x, GLfloat y, GLfloat radius)
{
	int i;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0,0);
	for(i = 0;i<128; i+=2){
		glVertex2f(unit_circle[i]*radius, unit_circle[i+1]*radius);
	}
	glEnd();
}

void draw_donut(GLfloat x, GLfloat y, GLfloat inner_r, GLfloat outer_r)
{
	int i;
	glBegin(GL_TRIANGLE_STRIP);
	for(i = 0;i<128; i+=2){
		glVertex2f(unit_circle[i]*inner_r, unit_circle[i+1]*inner_r);
		glVertex2f(unit_circle[i]*outer_r, unit_circle[i+1]*outer_r);
	}
	glEnd();
}

void draw_simple_box(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	glBegin(GL_QUADS);
		glVertex2d(x, y);
		glVertex2d(x, y + h);
		glVertex2d(x + w, y + h );
		glVertex2d(x + w, y);
	glEnd();
}

void draw_simple_circle(GLfloat x, GLfloat y, GLfloat radius,GLfloat rot)
{
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
	glColor3f(1,1,1);
	draw_simple_box(0, 0, radius, 5);
	glPopMatrix();
}

void draw_polygon(int count, cpVect *verts, Color lineColor, Color fillColor)
{
#if CP_USE_DOUBLES
	glVertexPointer(2, GL_DOUBLE, 0, verts);
#else
	glVertexPointer(2, GL_FLOAT, 0, verts);
#endif
	
	if(fillColor.a > 0){
		glColor_from_color(fillColor);
		glDrawArrays(GL_TRIANGLE_FAN, 0, count);
	}
	
	if(lineColor.a > 0){
		glColor_from_color(lineColor);
		glDrawArrays(GL_LINE_LOOP, 0, count);
	}
}

void draw_segment(cpVect a, cpVect b, cpFloat w, Color lineColor)
{
	glColor_from_color(lineColor);
	glBegin(GL_QUADS);
	glVertex2f(a.x, a.y - w);
	glVertex2f(a.x, a.y + w);
	glVertex2f(b.x, b.y + w);
	glVertex2f(b.x, b.y - w);
	glEnd();
	//draw_line(a.x, a.y, b.x, b.y, 10);
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
	draw_line(circle->tc.x, circle->tc.y, circle->tc.x - vel.x/32, circle->tc.y - vel.y/32, 32); //40 = 4 * radius
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
void draw_hp(cpFloat x, cpFloat y, cpFloat w, cpFloat h, cpFloat p)
{
	float border;
	glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
	p = (p < 1 ? (p > 0 ? p : 0) : 1);

	/* outer edge */
	glColor4f(1, 1, 1, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_simple_box(x, y, w, h);

	/* inner edge */

	border = 0.1 * (w > h ? h : w);
	glColor3f(0, 0, 0);
	draw_simple_box(x + border, y + border, w - border * 2, h - border * 2);

	/* hp bar */
	border *= 2;
	glColor3f(1-((p*p)*(p*p))*((p*p)*(p*p)), 0.8-(1-p)*(1-p)*0.8 + 0.1, 0.1);
	if (w > h) {
		draw_simple_box(x + border, y + border, (w - border * 2) * p, h - border * 2);
	} else {
		draw_simple_box(x + border, y + border, w - border * 2, (h - border * 2) * p);
	}

	glPopAttrib();
}
