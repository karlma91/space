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
  for(i = 0; i < size-3; i++){
    array[i] = sinf( 2*M_PI*i / (size-4));
    array[i+1] = cosf( 2*M_PI*i / (size-4));
    i++;
  }
  array[size-2] = 0.0f;
  array[size-1] = 0.0f;
}

void drawCircle(cpVect center, cpFloat angle, cpFloat radius, Color fill, Color line)
{
	int len;
	GLfloat *array;
	if(radius < 10){
		len = CIRCLE_SMALL/2;
		array = circle_8;
	}else if(radius < 100){
		len = CIRCLE_MEDIUM;
		array = circle_16;
	}else if(radius < 1000){
		len = CIRCLE_BIG/2;
		array = circle_64;
	}else{
		len = CIRCLE_EXTRA/2;
		array = circle_128;
	}
	
  glVertexPointer(2, GL_FLOAT, 0, array);
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
                        glDrawArrays(GL_LINE_STRIP, 0, len);
                }
    } glPopMatrix();
}