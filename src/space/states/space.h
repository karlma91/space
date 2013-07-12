#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "../level.h"
#include "../../engine/input/joystick.h"
#include "../../engine/graphics/camera.h"

void space_init();

/* chipmunk physics space */
extern cpSpace *space;

extern int multiplayer;
extern level *currentlvl;
extern joystick *joy_left, *joy_right;

extern int space_rendering_map;

/* init new level */
void space_init_level(int space_station, int deck);

float extern getGameTime();
extern int getPlayerScore();

extern void drawStars();

void space_start_demo();
void space_start_multiplayer();

#endif /* SPACE_H_ */
