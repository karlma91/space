#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "../level/spacelvl.h"
#include "we_input.h"
#include "we_graphics.h"
#include "../solarsystem.h"

void space_init(void);

extern int multiplayer;
extern spacelvl *currentlvl;
extern joystick *joy_p1_left, *joy_p1_right;
extern joystick *joy_p2_left, *joy_p2_right;

extern view *view_p1, *view_p2;

float extern getGameTime(void);
extern int getArcadeScore(void);

void space_return(void *unused);

//extern void drawStars(void);

void space_restart_level(void *unused);

#endif /* SPACE_H_ */
