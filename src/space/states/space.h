#ifndef SPACE_H_
#define SPACE_H_

#include "chipmunk.h"
#include "../level.h"
#include "we_input.h"
#include "we_graphics.h"

void space_init(void);

extern int multiplayer;
extern level *currentlvl;
extern joystick *joy_p1_left, *joy_p1_right;
extern joystick *joy_p2_left, *joy_p2_right;

extern view *view_p1, *view_p2;

extern particle_system *current_particles;

/* init new level */
void space_init_level(int space_station, int deck);

float extern getGameTime(void);
extern int getPlayerScore(void);

extern void drawStars(void);

void space_start_demo(int station, int deck);
void space_start_multiplayer(int station, int deck);

void space_restart_level(void);
void space_next_level(void);

#endif /* SPACE_H_ */
