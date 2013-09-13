#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "../level.h"
#include "we_input.h"
#include "we_graphics.h"

void space_init(void);

/* chipmunk physics space */
extern cpSpace *space;

extern int multiplayer;
extern level *currentlvl;
extern joystick *joy_p1_left, *joy_p1_right;
extern joystick *joy_p2_left, *joy_p2_right;

extern int space_rendering_map;
extern cpBodyVelocityFunc space_velocity;
extern particle_system *parti;

/* init new level */
void space_init_level(int space_station, int deck);

float extern getGameTime(void);
extern int getPlayerScore(void);

extern void drawStars(void);

void space_start_demo(void);
void space_start_multiplayer(void);

void space_restart_level(void);
void space_next_level(void);

#endif /* SPACE_H_ */
