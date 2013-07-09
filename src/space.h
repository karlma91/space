#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "main.h"
#include "level.h"
#include "joystick.h"
#include "camera.h"

void space_init();

/* camera */
extern camera *current_camera;

/* chipmunk physics space */
extern cpSpace *space;

extern level *currentlvl;

extern joystick *joy_left, *joy_right;

extern int space_rendering_map;

/* init new level */
void space_init_level(int space_station, int deck);

float extern getGameTime();
extern int getPlayerScore();

extern void drawStars();

#endif /* SPACE_H_ */
