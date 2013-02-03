#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "main.h"

extern state state_space;

/* camera */
extern float cam_center_x;
extern float cam_center_y;
extern float cam_zoom;

/* level data */
extern int level_height;
extern int level_left;
extern int level_right;

/* chipmunk physics space */
extern cpSpace *space;


#endif /* SPACE_H_ */
