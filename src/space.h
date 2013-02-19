#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "main.h"
#include "level.h"

extern state state_space;

/* camera */
extern float cam_center_x;
extern float cam_center_y;
extern float cam_zoom;


/* chipmunk physics space */
extern cpSpace *space;

extern level *currentlvl;

/* init new level */
void space_init_level(int space_station, int deck);

float extern getGameTime();

#endif /* SPACE_H_ */
