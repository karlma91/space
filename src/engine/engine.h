#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include "SDL.h"
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

#define SLEEP_TIME 1

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

extern SDL_Joystick *accelerometer;

extern LList active_fingers;

#define GAME_HEIGHT 1200
extern int GAME_WIDTH; // expected minimum width = 4:3 -> 1600
extern float ASPECT_RATIO; // 2:1 <-> 8:5 <-> 4:3

/* The delta time */
extern float dt;
extern int mdt;

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

extern unsigned char *keys;

extern char fps_buf[];
/* current state in use */
extern state *currentState;

extern button btn_fullscreen;

extern camera default_camera;
extern camera *current_camera;

extern void main_stop();

void normalized2game(float *x, float *y);

#endif /* MAIN_H_ */
