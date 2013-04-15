/* header */
#include "main.h"

/* standard c-libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* SDL and OpenGL */
#include "SDL.h"
#include "SDL_opengl.h"

/* Chipmunk physics library */
#include "chipmunk.h"

/* ini loader lib */
#include "ini.h"

#include "statesystem.h"

/* drawing code */
#include "draw.h"
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

configuration config;

static int handler(void* config, const char* section, const char* name,
		const char* value) {
	configuration* pconfig = (configuration*) config;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("video", "fullscreen")) {
		pconfig->fullscreen = atoi(value);
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

static int init_config() {
	if (ini_parse("bin/config.ini", handler, &config) < 0) {
		printf("Could not load 'bin/config.ini'\n");
		return 1;
	}
	fprintf(stderr, "Config loaded from 'bin/config.ini': fullscreen=%d\n",
			config.fullscreen);
	return 0;
}

static void initGL() {
	// Create an OpenGL context associated with the window.
	glcontext = SDL_GL_CreateContext(window);

	// TODO fix AA: find out when and where to store the attributes
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    //for antialiasing
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); //TODO fix tile edges when AA is activated
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); //TODO read AA-settings from config file


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    /* set the clear color to dark blue */
	glClearColor(0, 0.08, 0.15, 1);

	/* print gl info */
	fprintf(stderr, "GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	fprintf(stderr, "GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	fprintf(stderr, "GL_VERSION: %s\n", glGetString(GL_VERSION));
	fprintf(stderr, "GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLdouble W_2 = WIDTH / 2;
	GLdouble H_2 = HEIGHT / 2;
	glOrtho(-W_2, W_2, -H_2, H_2, 1, -1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, W, H);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_MULTISAMPLE);
}

static void setAspectRatio() {
	WIDTH = 1920;
	HEIGHT = (1.0f * H / W) * WIDTH;
}

static int window_init() {
	Uint32 flags = (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)
			| (SDL_WINDOW_FULLSCREEN * config.fullscreen);
	fprintf(stderr, "DEBUG - creating window\n");
	window = SDL_CreateWindow("SPACE", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, W, H, flags);
	displayRenderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (window == NULL ) {
		fprintf(stderr, "ERROR - could not create window!\n");
		SDL_Quit();
		return 1;
	}

	return 0;
}

SDL_Rect fullscreen_dimensions;
static int main_init() {
	fprintf(stderr, "DEBUG - init_config\n");
	init_config();
	fprintf(stderr, "DEBUG - init_config done!\n");

	fprintf(stderr, "DEBUG - SDL_init\n");
	if (SDL_Init(SDL_INIT_VIDEO)) {
		SDL_GetError();
		return 1;
	}
	fprintf(stderr, "DEBUG - SDL_init done!\n");

	SDL_GetDisplayBounds(0, &fullscreen_dimensions);

	if (config.fullscreen) {
		W = fullscreen_dimensions.w;
		H = fullscreen_dimensions.h;
	} else {
		W = config.width;
		H = config.height;
	}

	setAspectRatio();

	if (window_init())
		return 1;

	// random seed
	srand(time(NULL ));

	int error;

	//FIXME SDL2 port break
	initGL();

	/* preload textures */
	texture_init(); //FIXME SDL2 port break

	error = draw_init();
	if (error) {
		return error;
	}

	font_init();
	particles_init();

	/* load levels */
	error = level_init();
	if (error) {
		return error;
	}

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
			printf("%s\n", fps_buf);
			frames = 0;
			fps = 0;
		}

		deltaTime = deltaTime > 0.25 ? 0.25 : deltaTime;
		dt = deltaTime;
		mdt = dt * 1000;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		statesystem_update();
		statesystem_draw();

		int gl_error = glGetError();
		if (gl_error) fprintf(stderr,"main.c: %d  GL_ERROR: %d\n",__LINE__,gl_error); //TODO REMOVE

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
	level_destroy();
	//cpSpaceFreeChildren(space);

	/* destroy states */
	statesystem_destroy();

	draw_destroy();
	font_destroy();

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_DeleteContext(glcontext);

	SDL_DestroyRenderer(displayRenderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

int main(int argc, char *args[]) {
	main_init();
	main_run();
	main_destroy();

	return 0;
}

int main_stop() {
	main_running = 0;

	return main_running;
}
