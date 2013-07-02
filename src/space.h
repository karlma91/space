#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "main.h"
#include "level.h"
#include "statesystem.h"
#include "joystick.h"

extern STATE_ID STATE_SPACE;

void space_init();

/* camera */
extern float cam_center_x;
extern float cam_center_y;
extern float cam_zoom;

extern int cam_left;
extern int cam_right;

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
