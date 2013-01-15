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
GLfloat array[CIRCLE_MAX_RES];

static GLfloat colors1[CIRCLE_EXTRA/2*3];
static int i, j, len;
unsigned texture;

GLuint c_8,c_16,c_64,c_128;

float cam_zoom = 1;

#define DEBUG fprintf(stderr, "line: %d", __LINE__);

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

        
	 //TODO: load alpha BMP for FX
		DEBUG;
        SDL_Surface *surface;
		DEBUG;
        if((surface = SDL_LoadBMP("textures/dot_32bit.bmp"))==NULL){
		printf("Unable to loade texture");
		DEBUG;
		return;
	}
        glGenTextures(1,&texture);
	DEBUG;

        //bind texture object
        glBindTexture(GL_TEXTURE_2D,texture);
	DEBUG;
        // set texture's stretching properties
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	DEBUG;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	DEBUG;
	// surface->format->Amask = 0XFF000000;
	DEBUG;
	//surface->format->Ashift = 24;
	DEBUG;
        glTexImage2D(GL_TEXTURE_2D, 0, 3, surface->w, surface->h, 0, 
	GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
	DEBUG;

	SDL_FreeSurface(surface);
	DEBUG;
        //.....
        

}



void draw_texture(unsigned texture, cpVect a, cpVect dir, int w)
{
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glColor3f(1.0f,1.0f,1.0f);
         glPushMatrix();
      glTranslatef(a.x, a.y, 0.0f);
      // glRotatef(cpvtoangle(dir), 0.0f, 0.0f, 1.0f);
      glScalef(100,100,1);
      glBegin(GL_QUADS);
                glTexCoord2d(0, 0); glVertex2d(0, 0);
                glTexCoord2d(1, 0); glVertex2d(1, 0);
                glTexCoord2d(1, 1); glVertex2d(1, 1);
                glTexCoord2d(0, 1); glVertex2d(0, 1);
          glEnd();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
}


void draw_destroy()
{
	glDeleteLists(c_8, 1); 
	glDeleteLists(c_16, 1); 
	glDeleteLists(c_64, 1); 
	glDeleteLists(c_128, 1); 

        //TODO release texture resources
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

  draw_circle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), 10,cam_zoom, RGBAColor(0.80f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
}
void draw_boxshape(cpShape *shape)
{
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
