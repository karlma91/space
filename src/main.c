/* header */
#include "main.h"
#include "waffle_utils.h"

/* standard c-libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* SDL and OpenGL */
#include "SDL.h"

/* Chipmunk physics library */
#include "chipmunk.h"

/* ini loader lib */
#if !(TARGET_OS_IPHONE || __ANDROID__)
#include "ini.h"
#endif

#include "statesystem.h"

/* drawing code */
#include "draw.h"	//opengl included in draw.h
#include "font.h"
#include "particles.h"

#include "level.h"

/* Game states */
#include "menu.h"
#include "space.h"
#include "gameover.h"
#include "levelselect.h"

#define FPS_SLEEP_TIME 16
static float fps;
static float frames;

unsigned int KEY_UP_1 = SDL_SCANCODE_W;
unsigned int KEY_UP_2 = SDL_SCANCODE_UP;
unsigned int KEY_LEFT_1 = SDL_SCANCODE_A;
unsigned int KEY_LEFT_2 = SDL_SCANCODE_LEFT;
unsigned int KEY_RIGHT_1 = SDL_SCANCODE_D;
unsigned int KEY_RIGHT_2 = SDL_SCANCODE_RIGHT;
unsigned int KEY_DOWN_1 = SDL_SCANCODE_S;
unsigned int KEY_DOWN_2 = SDL_SCANCODE_DOWN;

unsigned int KEY_RETURN_1 = SDL_SCANCODE_SPACE;
unsigned int KEY_RETURN_2 = SDL_SCANCODE_RETURN;
unsigned int KEY_ESCAPE = SDL_SCANCODE_ESCAPE;

/* definition of external variables */
char fps_buf[15];
int WIDTH;
int HEIGHT;

int W, H;

float dt;
float mdt;
unsigned char *keys;
state *currentState;

SDL_GLContext glcontext;

SDL_Window *window;
SDL_Renderer *displayRenderer;

static int main_running = 1;

configuration config = {
		.fullscreen = 1,
	    .arcade = 1,
	    .arcade_keys = 1,
	    .width = 1920,
	    .height = 1200
};

#if !(TARGET_OS_IPHONE)
static int handler(void* config, const char* section, const char* name,
		const char* value) {
	configuration* pconfig = (configuration*) config;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("video", "fullscreen")) {
		pconfig->fullscreen = atoi(value);
  }else if (MATCH("video", "arcade")) {
	  pconfig->arcade = atoi(value);
  } else if (MATCH("video", "arcade_keys")) {
	  pconfig->arcade_keys = atoi(value);

	} else if (MATCH("video", "width")) {
		pconfig->width = atoi(value);
	} else if (MATCH("video", "height")) {
		pconfig->height = atoi(value);
	} else if (MATCH("keyboard", "key_left")) {
		pconfig->key_left = atoi(value);
	} else if (MATCH("keyboard", "key_up")) {
		pconfig->key_up = atoi(value);
	} else if (MATCH("keyboard", "key_right")) {
		pconfig->key_right = atoi(value);
	} else if (MATCH("keyboard", "key_down")) {
		pconfig->key_down = atoi(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}
#endif

static int init_config()
{
#if !(TARGET_OS_IPHONE || __ANDROID__)
	if (ini_parse("bin/config.ini", handler, &config) < 0) {
		SDL_Log("Could not load 'bin/config.ini'\n");
		return 1;
	}
	//fprintf(stderr,"Config loaded from 'bin/config.ini': fullscreen=%d\n", config.fullscreen);
#else

#endif

	if (config.arcade_keys) {
		KEY_UP_2 = SDL_SCANCODE_UP;
		KEY_UP_1 = SDL_SCANCODE_W;
		KEY_LEFT_2 = SDL_SCANCODE_LEFT;
		KEY_LEFT_1 = SDL_SCANCODE_A;
		KEY_RIGHT_2 = SDL_SCANCODE_RIGHT;
		KEY_RIGHT_1 = SDL_SCANCODE_D;
		KEY_DOWN_2 = SDL_SCANCODE_DOWN;
		KEY_DOWN_1 = SDL_SCANCODE_S;

		KEY_RETURN_1 = SDL_SCANCODE_K;
		KEY_RETURN_2 = SDL_SCANCODE_G;
		KEY_ESCAPE = SDL_SCANCODE_ESCAPE;
	}

	return 0;
}


static void initGL() {

	// Create an OpenGL context associated with the window.
	glcontext = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, glcontext);

	SDL_GL_SetSwapInterval(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    /* set the clear color to dark blue */
	glClearColor(0, 0.08, 0.15, 1);

	/* print gl info */
	SDL_Log("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	SDL_Log("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	SDL_Log("GL_VERSION: %s\n", glGetString(GL_VERSION));
	////SDL_Log("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));

#if !(GLES2)
    glEnableClientState( GL_VERTEX_ARRAY );	 // Enable Vertex Arrays
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );	// Enable Texture Coord Arrays
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLdouble W_2 = WIDTH / 2;
	GLdouble H_2 = HEIGHT / 2;
	glOrtho(-W_2, W_2, -H_2, H_2, 1, -1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, W, H);
	glClearColor(0,0,0, 1);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_MULTISAMPLE);
#else

#endif
}

static void setAspectRatio() {
	WIDTH = 1920;
	HEIGHT = (1.0f * H / W) * WIDTH;
}

static int window_init() {
	Uint32 flags = (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)
			| (SDL_WINDOW_FULLSCREEN * config.fullscreen);
	SDL_Log("DEBUG - creating window\n");

	window = SDL_CreateWindow("SPACE", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, W, H, flags);

	if (window == NULL ) {
		SDL_Log("ERROR - could not create window!\n");
		SDL_Quit();
		return 1;
	}

	return 0;
}

SDL_Rect fullscreen_dimensions;
static int main_init() {
	waffle_init();

	SDL_Log("DEBUG - init_config\n");
	init_config();
	SDL_Log("DEBUG - init_config done!\n");

	SDL_Log("DEBUG - SDL_init\n");
	if (SDL_Init(SDL_INIT_VIDEO)) {
		SDL_GetError();
		return 1;
	}
	SDL_Log("DEBUG - SDL_init done!\n");

	SDL_GetDisplayBounds(0, &fullscreen_dimensions);

	SDL_ShowCursor(SDL_DISABLE);

	if (config.fullscreen) {
		W = fullscreen_dimensions.w;
		H = fullscreen_dimensions.h;
	} else {
		W = config.width;
		H = config.height;
	}

	setAspectRatio();

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	if (window_init())
		return 1;

	//FIXME SDL2 port break
	initGL();

SDL_Log("DEBUG - initGL done!\n");

	// random seed
	srand(time(NULL ));

	/* preload textures */
	texture_init(); //FIXME SDL2 port break

SDL_Log("DEBUG - texture_init done!\n");

	int error;
	/* load levels */
	error = draw_init();
	if (error) {
		return error;
	}

SDL_Log("DEBUG - draw_init done!\n");

	error = level_init();
	if (error) {
		return error;
	}

SDL_Log("DEBUG - level_init done!\n");

	font_init();
	particles_init();


	/* init states */
	statesystem_init();
	statesystem_set_state(STATESYSTEM_MENU);
	return 0;
}

static int main_run() {
	SDL_Event event;
	Uint32 thisTime = 0;
	Uint32 lastTime;
	float deltaTime = 0.0f;

	statesystem_set_state(STATESYSTEM_MENU);

	lastTime = SDL_GetTicks();

	//START GAME
	if(config.arcade){
		SDL_Log("start %s\n", "999");
	}

	while (main_running) {

		thisTime = SDL_GetTicks();
		deltaTime = (float) (thisTime - lastTime) / 1000.0f;
		lastTime = thisTime;

		SDL_PumpEvents();
		keys = SDL_GetKeyboardState(NULL );

		frames += deltaTime;
		fps++;
		if (frames >= 1) {
			sprintf(fps_buf, "%.2f FPS", fps);
			SDL_Log("%s\n", fps_buf);
			frames = 0;
			fps = 0;
		}

		deltaTime = deltaTime > 0.25 ? 0.25 : deltaTime;
		dt = deltaTime;
		mdt = dt * 1000;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if !GLES2
		glLoadIdentity();
#endif

		statesystem_update();
		statesystem_draw();

		int gl_error = glGetError();
		if (gl_error) SDL_Log("main.c: %d  GL_ERROR: %d\n",__LINE__,gl_error); //TODO REMOVE

		SDL_GL_SwapWindow(window);

		//input handler
		if (keys[SDL_SCANCODE_F12]) {
			main_stop();
		}

		if (keys[SDL_SCANCODE_F] && keys[SDL_SCANCODE_LCTRL]) {
			config.fullscreen ^= 1; // Toggle fullscreen
			if (SDL_SetWindowFullscreen(window, config.fullscreen)) {
				SDL_GetError();
				config.fullscreen ^= 1; // Re-toggle
			} else {
				keys[SDL_SCANCODE_F] = 0;
			}
		}

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				main_stop();
				break;
			case SDL_WINDOWEVENT_RESIZED:
				//if (config.fullscreen) break;
				//if (config.fullscreen) {
				//	W = fullscreen_dimensions.w;
				//	H = fullscreen_dimensions.h;
				//} else {
				W = event.window.data1;
				H = event.window.data2;
				//}
				window_init();
				glViewport(0, 0, W, H);
				setAspectRatio();
				break;
			}
		}

		//not use 100% of cpu
		if (keys[SDL_SCANCODE_Z])
			SDL_Delay(14);
		else if (keys[SDL_SCANCODE_C])
			SDL_Delay(50);

		static int FPS_EXTRA_SLEEP = 0;
		if (FPS_EXTRA_SLEEP >= 2)
			FPS_EXTRA_SLEEP = 0;
		else
			++FPS_EXTRA_SLEEP;

		Uint32 sleep_delta = SDL_GetTicks() - thisTime;
		if (sleep_delta < FPS_SLEEP_TIME) {
			SDL_Delay((FPS_SLEEP_TIME) - sleep_delta + (FPS_EXTRA_SLEEP != 0));
		}
	}
	return 0;
}

static int main_destroy() {
SDL_Log("DEBUG - SDL_destroy\n");
	level_destroy();
	//cpSpaceFreeChildren(space);

	/* destroy states */
	statesystem_destroy();

	draw_destroy();
	font_destroy();

	waffle_destroy();

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_MakeCurrent(NULL, NULL);
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

int main(int argc, char *args[]) {
	if (main_init() == 0) {
		main_run();
	}
	//main_destroy();

	return 0;
}

int main_stop() {
	main_running = 0;

	return main_running;
}
