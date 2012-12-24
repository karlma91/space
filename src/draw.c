#include "draw.h"
#include <string.h>

//local function
static void init_array(int size,  GLuint *index);
GLfloat array[CIRCLE_MAX_RES];

static GLfloat colors1[CIRCLE_EXTRA/2*3];
static int i, j, len;

GLuint c_8,c_16,c_64,c_128;


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
}
void draw_destroy(){
	glDeleteLists(c_8, 1); 
	glDeleteLists(c_16, 1); 
	glDeleteLists(c_64, 1); 
	glDeleteLists(c_128, 1); 
}

static void init_array(int size, GLuint *index) {
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

void drawCircle(cpVect center, cpFloat angle, cpFloat radius,cpFloat scale, Color fill, Color line)
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
