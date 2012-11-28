#include "draw.h"

static GLfloat circle_8[CIRCLE_SMALL];
static GLfloat circle_16[CIRCLE_MEDIUM];
static GLfloat circle_64[CIRCLE_BIG];
static GLfloat circle_128[CIRCLE_EXTRA];

static void init_array(int size, GLfloat *array);

void draw_init(){
	init_array( CIRCLE_SMALL, circle_8);
	init_array(CIRCLE_MEDIUM, circle_16);
	init_array(CIRCLE_BIG, circle_64);
	init_array(CIRCLE_EXTRA, circle_128);
}

static void 
init_array(int size, GLfloat *array)
{
  int i;
  array[0]=0.0f;
  array[1]=0.0f;
  for(i = 0; i < size-3; i++){
    array[i+2] = sinf( 2*M_PI*i / (size-6));
    array[i+3] = cosf( 2*M_PI*i / (size-6));
    i++;
  }
  array[size-2] = 0.0f;
  array[size-1] = 0.0f;
}

void drawCircle(cpVect center, cpFloat angle, cpFloat radius, Color fill, Color line)
{
	int len,i;
	GLfloat *array;
	if(radius < 10){
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
  GLfloat colors1[len*3]; //= {fill.r, fill.g, fill.b};
  colors1[0] = 1.0f;
  colors1[1] = 0.0f;
  colors1[2] = 0.0f;
  for(i=3;i<len*3;i+=3){
	colors1[i]= 0.0f;
	colors1[i+1]= 0.0f;
	colors1[i+2]= 0.0f;
  }
  
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
    /*          
    if(line.a > 0){
     glColor_from_color(line);
     glDrawArrays(GL_LINE_STRIP, 0, len);
    }
    */
    } glPopMatrix();
  
  glDisableClientState(GL_COLOR_ARRAY);
}