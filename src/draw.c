#include "draw.h"
//local function
static void init_array(int size,  GLuint *index);

static GLfloat colors1[CIRCLE_EXTRA/2*3];
int i, len;

GLuint c_8,c_16,c_64,c_128;


void draw_init(){
	init_array( CIRCLE_SMALL, &c_8);
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

static void 
init_array(int size, GLuint *index)
{
  GLfloat array[size];
  array[0]=0.0f;
  array[1]=0.0f;
  for(i = 2; i < size-1; i++){
    array[i] = sinf( 2*M_PI*i / (size-6));
    array[i+1] = cosf( 2*M_PI*i / (size-6));
    i++;
  }
  array[size-2] = 0.0f;
  array[size-1] = 0.0f;
  
  *index = glGenLists(1);
  glVertexPointer(2, GL_FLOAT, 0, array);
  // compile the display list, store a triangle in it
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
	  glBegin(GL_LINES);
		glVertex2f(0,0);
		glVertex2f(1,0);
	  glEnd();
	glPopMatrix();
}


TTF_Font* loadfont(char* file, int ptsize)
  {
    TTF_Font* tmpfont;
    tmpfont = TTF_OpenFont(file, ptsize);
    if (tmpfont == NULL){
      printf("Unable to load font: %s %s \n", file, TTF_GetError());
      // Handle the error here.
    }
    return tmpfont;
  }


void RenderText(TTF_Font *Font, Color color,double X, double Y, char *Text)
{

    SDL_Color c = {(int)(color.r*255),(int)(color.g*255),(int)(color.b*255),255};
	/*Create some variables.*/
	SDL_Surface *Message = TTF_RenderText_Blended(Font, Text, c);
	unsigned Texture = 0;
 
	/*Generate an OpenGL 2D texture from the SDL_Surface*.*/
	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Message->w, Message->h, 0, GL_BGRA,
	             GL_UNSIGNED_BYTE, Message->pixels);
 
	/*Draw this texture on a quad with the given xyz coordinates.*/
	glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2d(X, Y+Message->h);
		glTexCoord2d(1, 0); glVertex2d(X+Message->w, Y+Message->h);
		glTexCoord2d(1, 1); glVertex2d(X+Message->w, Y);
		glTexCoord2d(0, 1); glVertex2d(X, Y);
	glEnd();
 
	/*Clean up.*/
	glDeleteTextures(1, &Texture);
	SDL_FreeSurface(Message);
}
