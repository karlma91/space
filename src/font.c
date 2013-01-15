#include "string.h"
#include "font.h"
#include "SDL_opengl.h"

// initialize global variables
int font_text_align = TEXT_CENTER;
float font_text_size = 12;
float font_text_angle = 0;

// local variables
static int i,j;

static GLuint firstCharList;

//inefficient 7-segment display style for digits
//TODO: convert to redered list
const GLint digits[] = {0x3F,0x21,0x5B,0x73,0x65,0x76,0x7E,0x23,0x7F,0x67};
const GLfloat digits_x[] = {0.5f,0.5f,-0.5f,-0.5f,-0.5f, 0.5f,0.5f,-0.5f};
const GLfloat digits_y[] = {0,0.5, 0.5, 0,-0.5,-0.5,0, 0};

static void drawDigit(int d)
{
	j = 0;
	glBegin(GL_LINES);
	while (d && j < 7) {
		if (d & 0x1) {
			glVertex2f(digits_x[j],digits_y[j]);
			j++;
			glVertex2f(digits_x[j],digits_y[j]);
		} else j++;
		d >>= 1;
	}
	glEnd();
}

const GLfloat letters[26][14] = {
	{0.5,-0.5,	0.5,0.5,	-0.5,0.5,	-0.5,	-0.5,	-0.5,0	,0.5,0}, //A
	{0.4,0,	0.4,0.5,	-0.5,0.5,	-0.5,-0.5,	0.5,-0.5,	0.5,0,	-0.5,0}, //B
	{0.5,0.5,	-0.5,0.5,	-0.5,-0.5,	0.5,-0.5}, //C
	{0.25,0.5,	-0.5,0.5,	-0.5,-0.5,	0.25,-0.5, 0.25,0.5}, //D
	{0.5,0.5,	-0.5,0.5,	-0.5,0,	0.5,0,	-0.5,0,	-0.5,-0.5,	0.5,-0.5}, //E
	{0.5,0.5,	-0.5,0.5,	-0.5,0,	0.5,0,	-0.5,0,	-0.5,-0.5}, //F
	{0.5,0.5,	-0.5,0.5,	-0.5,-0.5,	0.5,-0.5,	0.5,0,	0,0}, //G
	{-0.5,0.5,	-0.5,-0.5,	-0.5,0,	0.5,0,	0.5,-0.5,	0.5,0.5}, //H
	{0,0.5,	0,-0.5}, //I
	{0,0.5,	0,-0.5, -0.5,-0.5}, //J
	{0.5,0.5,	-0.5,0,	-0.5,0.5,	-0.5,-0.5,	-0.5,0,	0.5,-0.5}, //K
	{0,0.5,	0,-0.5, 0.5,-0.5}, //L
	{-0.5,-0.5,	-0.5,0.5, 0,-0.5,	0.5,0.5,	0.5,-0.5}, //M
	{-0.5,-0.5,	-0.5,0.5, 0.5,-0.5,	0.5,0.5}, //N
	{0.5,0.5,	-0.5,0.5,	-0.5,-0.5,	0.5,-0.5, 0.5,0.5}, //O
	{-0.5,0,	0.5,0,	0.5,0.5,	-0.5,0.5,	-0.5,	-0.5}, //P
	{0.5,-0.5,	0.5,0.5,	-0.5,0.5,	-0.5,-0.5,	0.5,-0.5, 0,0}, //Q
	{0.5,-0.5, -0.5,0,	0.5,0,	0.5,0.5,	-0.5,0.5,	-0.5,	-0.5}, //R
	{0.5,0.5,	-0.5,0.5,	-0.5,0,	0.5,0,	0.5,-0.5,	-0.5,-0.5}, //S
	{-0.5,0.5,	0,0.5,	0,-0.5,	0,0.5,	0.5,0.5}, //T
	{-0.5,0.5,	-0.5,-0.5,	0.5,-0.5,	0.5,0.5}, //U
	{-0.5,0.5,	0,-0.5,	0.5,0.5}, //V
	{-0.5,0.5,	-0.5,-0.5, 0,0,	0.5,-0.5,	0.5,0.5}, //W
	{0.5,0.5,	-0.5,-0.5,	0,0,	-0.5,0.5,	0.5,-0.5}, //X
	{0,0,	0,-0.5,	0,0	,0.5,0.5,	0,0,	-0.5,0.5}, //Y
	{-0.5,0.5,	0.5,0.5,	-0.5,-0.5,	0.5,-0.5}, //Z
};
//////////////////////////////////A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z
const GLint letters_points[26] = {7,7,4,5,7,6,6,6,2,3,6,3,5,4,5,5,6,6,6,5,4,3,5,5,6,4};

//TODO: swap lines with quads
static void drawLetter(int d)
{
	glVertexPointer(2, GL_FLOAT, 0, letters[d]);
	glDrawArrays(GL_LINE_STRIP, 0, letters_points[d]);
	
	//drawing corners
	//glDrawArrays(GL_POINTS, 0, letters_points[d]);
}

// symbol constants
const GLfloat MINUS[] = {-0.5,0,	0.5,0};
const GLfloat PLUS[] = {-0.5,0,	0.5,0,	0,-0.5,	0,0.5};
const GLfloat COMMA[] = {0,0,	-0.5,-0.5};
const GLfloat DOT[] = {-0.1,-0.5,	-0.1,-0.3,	0.1,-0.3,	0.1,-0.5, -0.1,-0.5f};

static void drawSymbol(char c)
{
	switch(c) {
		case '-':
			glVertexPointer(2, GL_FLOAT, 0, MINUS);
			glDrawArrays(GL_LINE_STRIP, 0, 2);
			break;
		case '+':
			glVertexPointer(2, GL_FLOAT, 0, PLUS);
			glDrawArrays(GL_LINES, 0, 4);
			break;
		case ',':
			glVertexPointer(2, GL_FLOAT, 0, COMMA);
			glDrawArrays(GL_LINES, 0, 2);
			break;
		case '.':
			glVertexPointer(2, GL_FLOAT, 0, DOT);
			glDrawArrays(GL_LINES, 0, 5);
			break;
		default:
			break;
			//do nothing
	}
}

void font_drawText(GLfloat x, GLfloat y, char* text)
{
	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	glRotatef(font_text_angle, 0, 0, 1);
	glScalef(font_text_size, font_text_size, 1);
	
	switch(font_text_align) {
		case TEXT_LEFT:
			glTranslatef(0.5f, 0, 0);
			glPushMatrix();
			break;
		case TEXT_CENTER:
			glTranslatef(-(strlen(text) * (CHAR_WIDTH + CHAR_SPACING))/2.0f + 0.5f + CHAR_SPACING/2, 0, 0.0f);
			break;
		case TEXT_RIGHT:
			glTranslatef(-(strlen(text) * (CHAR_WIDTH + CHAR_SPACING) - 0.5f  - CHAR_SPACING), 0,0);
	}
	
	i = 0;
	while(text[i] != '\0') {
		if (text[i] == '\n') {
			if (font_text_align == TEXT_LEFT) {
				glPopMatrix();
				glTranslatef(0, -(CHAR_WIDTH+CHAR_SPACING), 0);
				glPushMatrix();
				i++;
				continue;
			}
		} else {
			glCallList(firstCharList + text[i]);
		}
		i++;
		glTranslatef((CHAR_WIDTH + CHAR_SPACING), 0, 0);
	}
	
	if (font_text_align == TEXT_LEFT)
		glPopMatrix();
	
	glPopMatrix();
}

void init_text(char c)
{
	if (c >= '0' && c <= '9') {
		drawDigit(digits[c - '0']);
	} else if (c >= 'A' && c <= 'Z') {
		drawLetter(c - 'A');
	} else {
		drawSymbol(c);
	}
}

void font_init()
{
	firstCharList = glGenLists(256);
	
	char i = 1;
	do {
		glNewList(firstCharList + i,GL_COMPILE);
		init_text(i);
		glEndList();
	}	while (i++);
}

void font_destroy()
{
	glDeleteLists(firstCharList, 256); 
}
