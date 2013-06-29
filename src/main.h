#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include "SDL.h"

#include "joystick.h"

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

//TODO move to game.h
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

extern configuration config; //TODO move to game.h

/* State  struct */
typedef struct state_ state;
struct state_ {
	void (*init)();
	void (*update)();
	void (*render)();
	void (*destroy)();
	state *parentState;
};


extern joystick joy_left, joy_right;

extern int WIDTH;
extern int HEIGHT;
/* The delta time */
extern float dt;
extern int mdt;

extern unsigned char *keys;

extern char fps_buf[];
/* current state in use */
extern state *currentState;

extern void main_stop();

extern SDL_Window *window; //TODO TMP

extern Sint32 MOUSE_X_PRESSED, MOUSE_Y_PRESSED, MOUSE_X, MOUSE_Y;

extern unsigned int KEY_UP_1;
extern unsigned int KEY_UP_2;
extern unsigned int KEY_LEFT_1;
extern unsigned int KEY_LEFT_2;
extern unsigned int KEY_RIGHT_1;
extern unsigned int KEY_RIGHT_2;
extern unsigned int KEY_DOWN_1;
extern unsigned int KEY_DOWN_2;

extern unsigned int KEY_RETURN_1;
extern unsigned int KEY_RETURN_2;
extern unsigned int KEY_ESCAPE;

extern SDL_TouchFingerEvent touch[10];

#endif /* MAIN_H_ */
