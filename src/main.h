#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>

#define SLEEP_TIME 1

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

extern unsigned char *keys;

extern char fps_buf[];
/* current state in use */
extern state *currentState;

extern int main_stop();


#endif /* MAIN_H_ */
