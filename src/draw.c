#include "draw.h"

static GLfloat circle_8[CIRCLE_SMALL];
static GLfloat circle_16[CIRCLE_MEDIUM];
static GLfloat circle_64[CIRCLE_BIG];
static GLfloat circle_128[CIRCLE_EXTRA];

static GLfloat colors1[CIRCLE_EXTRA/2*3];

static void init_array(int size, GLfloat *array);

int i, len;

void draw_init(){
	init_array( CIRCLE_SMALL, circle_8);
	init_array(CIRCLE_MEDIUM, circle_16);
	init_array(CIRCLE_BIG, circle_64);
	init_array(CIRCLE_EXTRA, circle_128);
	
	colors1[0] = 0.9f;
	colors1[1] = 0.1f;
	colors1[2] = 0.1f;
	
	for(i=3;i<CIRCLE_EXTRA/2*3;i+=3){
		colors1[i]= 0.2f;
		colors1[i+1]= 0.1f;
		colors1[i+2]= 0.1f;
	}
}

static void 
init_array(int size, GLfloat *array)
{
  array[0]=0.0f;
  array[1]=0.0f;
  for(i = 2; i < size-1; i++){
    array[i] = sinf( 2*M_PI*i / (size-6));
    array[i+1] = cosf( 2*M_PI*i / (size-6));
    i++;
  }
  array[size-2] = 0.0f;
  array[size-1] = 0.0f;
}

void drawCircle(cpVect center, cpFloat angle, cpFloat radius, Color fill, Color line)
{
	GLfloat *array;
	if(radius < 20){
		len = CIRCLE_SMALL/2;
		array = circle_8;
	}else if(radius < 100){
		len = CIRCLE_MEDIUM/2;
		array = circle_16;
	}else if(radius < 1000){
		len = CIRCLE_BIG/2;
		array = circle_64;
	}else{
		len = CIRCLE_EXTRA/2;
		array = circle_128;
	}
	
  //TMP
  glEnableClientState(GL_COLOR_ARRAY);
  
  glVertexPointer(2, GL_FLOAT, 0, array);
  glColorPointer(3, GL_FLOAT, 0, colors1);
  
  glPushMatrix(); {
    glTranslatef(center.x, center.y, 0.0f);
    glRotatef(angle*180.0f/M_PI, 0.0f, 0.0f, 1.0f);
    glScalef(radius, radius, 1.0f);
    
    if(fill.a > 0){
      glColor_from_color(fill);
      glDrawArrays(GL_TRIANGLE_FAN, 0, len-1);
    }
             
    if(line.a > 0){
	  glColor_from_color(line);
	  glBegin(GL_LINES);
	    glVertex2f(0,0);
	    glVertex2f(1,0);
	  glEnd();
    }
    
    } glPopMatrix();
  
  glDisableClientState(GL_COLOR_ARRAY);
}