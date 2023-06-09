#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include "SDL.h"
//#include "SDL_net.h"
#include "input/button.h"
#include "data/llist.h"
#include "graphics/camera.h"

#if __IPHONEOS__
#define IS_APP 1
#define GOT_TOUCH 1
#endif

#if __ANDROID__
#define IS_APP 1
#define GOT_TOUCH 1
#include <jni.h>
#include <android/log.h>
#endif

#ifndef GOT_TOUCH
#define GOT_TOUCH 0
#endif

#ifndef IS_APP
#define IS_APP 0
#endif

#define SLEEP_TIME 1

/* General data structures */
typedef struct {
	float x, y;
} vect;

typedef struct {
	cpVect p;
	cpVect s;
} rect;

/* config struct */
typedef struct
{
    int fullscreen;
    int arcade;
    int arcade_keys;
    char key_left;
    char key_up;
    char key_right;
    char key_down;
    char key_space;
    unsigned int width;
    unsigned int height;
} configuration;

extern GLfloat engine_time;
extern configuration config;

/* State  struct */
typedef struct state_ state;
struct state_ {
	void (*init)(void);
	void (*update)(void);
	void (*render)(void);
	void (*destroy)(void);
	state *parentState;
};

extern SDL_Joystick *accelerometer;

#define GAME_HEIGHT 1200
extern int GAME_WIDTH; // expected minimum width = 4:3 -> 1600
extern float ASPECT_RATIO; // 2:1 <-> 8:5 <-> 4:3

extern SDL_Window *window;

/* The delta time */
extern float dt;
extern int mdt;

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

extern unsigned char *keys;

extern we_bool debug_logtime;

extern char fps_buf[];
/* current state in use */
extern state *currentState;

extern button btn_fullscreen;

extern void main_stop(void);

void normalized2game(float *x, float *y);

#endif /* MAIN_H_ */
