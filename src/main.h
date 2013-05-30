#ifndef MAIN_H_
#define MAIN_H_

#include "SDL.h"
#include <stdlib.h>

#if TARGET_OS_IPHONE

#endif

#if __ANDROID__
#include <jni.h>
#include <android/log.h>
#endif

#if TARGET_OS_IPHONE || __ANDROID__
#define GLES2 1
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

extern SDL_Window *window; //TODO TMP

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

#endif /* MAIN_H_ */
