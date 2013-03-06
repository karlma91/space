#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>

#define SLEEP_TIME 1

/* config struct */
typedef struct
{
    int fullscreen;
    char key_left;
    char key_up;
    char key_right;
    char key_down;
    char key_space;
} configuration;

extern configuration config;

/* State  struct */
typedef struct state_ state;
struct state_ {
	void (*init)();
	void (*update)();
	void (*render)();
	void (*destroy)();
	state *parentState;
};

extern int WIDTH;
extern int HEIGHT;
/* The delta time */
extern float dt;
extern float mdt;

extern unsigned char *keys;

extern char fps_buf[];
/* current state in use */
extern state *currentState;

extern int main_stop();


#endif /* MAIN_H_ */
