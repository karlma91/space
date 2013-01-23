#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"


#include <string.h>

//local function
static void init_array(int size,  GLuint *index);
static void loadTexture(char *tex);

GLfloat array[CIRCLE_MAX_RES];

static GLfloat colors1[CIRCLE_EXTRA/2*3];
static int i, j;
unsigned texture[10];
static int texC = 0;

Color rainbow_col[1536];

GLuint c_8,c_16,c_64,c_128;

#define DEBUG fprintf(stderr, "line: %d\n", __LINE__);

void draw_init(){
	init_array(CIRCLE_SMALL, &c_8);
	init_array(CIRCLE_MEDIUM, &c_16);
	init_array(CIRCLE_BIG, &c_64);
	init_array(CIRCLE_EXTRA,&c_128);
	
	colors1[0] = 0.9f;
	colors1[1] = 0.1f;
	colors1[2] = 0.1f;
	
	for(i=3;i<CIRCLE_EXTRA/2*3;i+=3){
		colors1[i]= 0.2f;
		colors1[i+1]= 0.1f;
		colors1[i+2]= 0.1f;
	}
	
	/* Photoshop Outer glow settings:
	 * Opacity 50%
	 * Spread 60%
	 * Size 32px
	 * Range 100%
	 * Jitter 0%
	 */
	loadTexture("textures/glowdot.bmp");
	loadTexture("textures/dot.bmp");
	
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
}

static void loadTexture(char *tex)
{
	SDL_Surface *surface;
	if((surface = SDL_LoadBMP(tex))==NULL){
		fprintf(stderr,"Unable to loade texture\n");
		return;
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
}



void draw_line(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, float w)
{
	glEnable(GL_TEXTURE_2D);
	
	glPushMatrix();
	glPushAttrib(GL_CURRENT_BIT);
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
	glPopAttrib();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void draw_line_strip(const GLfloat *strip, int l, float w)
{
	for(i = 0; i<l-2; i+=2){
		draw_line(strip[i],strip[i+1],strip[i+2],strip[i+3],w);
	}
}


void draw_destroy()
{
	glDeleteLists(c_8, 1); 
	glDeleteLists(c_16, 1); 
	glDeleteLists(c_64, 1); 
	glDeleteLists(c_128, 1); 
	
	//TODO! --> release texture resources
	//...
}

static void init_array(int size, GLuint *index) 
{
	array[0]=0.0f;
	array[1]=0.0f;
	
	for(i = 2; i < size-1; i++) {
		array[i] = sinf( 2*M_PI*i / (size-6));
		array[i+1] = cosf( 2*M_PI*i / (size-6));
		i++;
	}
	
	array[size-2] = 0.0f;
	array[size-1] = 0.0f;
	
	*index = glGenLists(1);
	glVertexPointer(2, GL_FLOAT, 0, array);
	
	glNewList(*index, GL_COMPILE);
	glDrawArrays(GL_TRIANGLE_FAN, 0, size/2);
	glEndList();
	// delete it if it is not used any more
	//glDeleteLists(index, 1); 
}

void draw_circle(cpVect center, cpFloat angle, cpFloat radius,cpFloat scale, Color fill, Color line)
{
	glColor_from_color(fill);
	int res;
	int size = radius*scale;
	
	if(size<=1){
		glBegin(GL_POINTS);
		glVertex2f(center.x, center.y);
		glEnd();
		return;
	}else if(size < 10){
		res = c_8;
	}else if(size < 100){
		res = c_16;
	}else if(size < 1000){
		res = c_64;
	}else{
		res = c_128;
	}
	glPushMatrix();
	glTranslatef(center.x, center.y, 0.0f);
	glRotatef(angle*180.0f/M_PI, 0.0f, 0.0f, 1.0f);
	glScalef(radius, radius, 1.0f);
	glCallList(res);
	glColor_from_color(line);
	/*
	 glBegin(GL_LINES);
	 glVertex2f(0,0);
	 glVertex2f(1,0);
	 glEnd();
	 */
	glPopMatrix();
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

void draw_segment(cpVect a, cpVect b, cpFloat width, Color lineColor)
{
	glColor_from_color(lineColor);
	glLineWidth(width);
	glBegin(GL_LINES);
	glVertex2f( a.x, a.y);
	glVertex2f( b.x, b.y);
	glEnd(); 
}

void draw_shape(cpShape *shape, void *unused)
{
	void (*functionPtr)(cpShape*);
	functionPtr = (void (*)(cpShape*))cpShapeGetUserData(shape);
	if(functionPtr != NULL){
		functionPtr(shape);
	}
}

void draw_ballshape(cpShape *shape)
{
	cpCircleShape *circle = (cpCircleShape *)shape;

	//printf("rand %f\n",1.0f*rand()/RAND_MAX);
	glColor3f(0.1f + sin(circle->tc.x/500)*0.8f,0.1f + cos(circle->tc.y/500)*0.8f,0.9f);
	
	cpVect vel = cpBodyGetVel(cpShapeGetBody(shape));
	
	draw_line(circle->tc.x, circle->tc.y, circle->tc.x + vel.x/16, circle->tc.y + vel.y/16, 40); //40 = 4 * radius

	glColor4f(1,1,1,0.4f);
	draw_line(circle->tc.x, circle->tc.y, circle->tc.x + vel.x/16, circle->tc.y + vel.y/16, 40);
	//draw_circle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), 10,cam_zoom, RGBAColor(0.80f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
}
void draw_boxshape(cpShape *shape)
{
	glLineWidth(2);
	cpPolyShape *poly = (cpPolyShape *)shape;
	draw_polygon(poly->numVerts, poly->tVerts,RGBAColor(0.80f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
}
void draw_segmentshape(cpShape *shape)
{
	cpSegmentShape *seg = (cpSegmentShape *)shape;
	draw_segment(seg->ta, seg->tb, seg->r, RGBAColor(0.80f, 0.107f, 0.05f,1.0f));
}

void draw_space(cpSpace *space)
{
	cpSpaceEachShape(space, draw_shape, NULL);
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

