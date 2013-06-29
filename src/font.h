#ifndef FONT_H_
#define FONT_H_


#include "constants.h"

// text alignment constants
#define TEXT_LEFT -1
#define TEXT_CENTER 0
#define TEXT_RIGHT 1
#define CHAR_WIDTH 1
#define CHAR_SPACING 0.5f

#ifndef SQUARE_FONT_H
#define SQUARE_FONT_H

// global variables
extern int font_text_align;
extern float font_text_size, font_text_angle;

// macros
#define setTextAlign(tmp_alignment) font_text_align=tmp_alignment;
#define setTextSize(tmp_size) font_text_size=tmp_size;
#define setTextAngle(tmp_angle) font_text_angle=tmp_angle;
#define setTextAngleRad(tmp_angle) font_text_angle=MATH_180PI*tmp_angle;

#endif

// prototypes
void font_drawText(float x, float y, char* text);
void font_init();
void font_destroy();

void init_text(char c);


#endif /* FONT_H_ */
