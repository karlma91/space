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
#include "SDL_main.h"

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


#if (GLES1)
#define glOrtho glOrthof

#define FPS_SLEEP_TIME 33
#else
#define FPS_SLEEP_TIME 16
#endif


Sint32 MOUSE_X_PRESSED = 0, MOUSE_Y_PRESSED = 0, MOUSE_X, MOUSE_Y;


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
SDL_Rect fullscreen_dimensions;

static int main_running = 1;

configuration config = {
		.fullscreen = 1,
	    .arcade = 1,
	    .arcade_keys = 1,
	    .width = 1920,
	    .height = 1200
};

#if !(TARGET_OS_IPHONE || __ANDROID__)
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

static void setAspectRatio() {
	SDL_ShowCursor(!config.fullscreen);

	if (config.fullscreen) {
		W = fullscreen_dimensions.w;
		H = fullscreen_dimensions.h;
		if (W < H) {
			int t = H;
			H = W;
			W = t;
		}
	} else {
		W = config.width;
		H = config.height;
	}

	WIDTH = 1920;
	HEIGHT = (1.0f * H / W) * WIDTH;
}

static void display_init()
{
	SDL_Log("DEBUG - SDL_init\n");
	if (SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("ERROR: SDL_INIT_VIDEO: %s", SDL_GetError());
		exit(-1);
	}
	SDL_Log("DEBUG - SDL_init done!\n");

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_SetHint("SDL_IOS_ORIENTATIONS", "LandscapeLeft LandscapeRight");

	Uint32 flags = SDL_WINDOW_OPENGL |
			       SDL_WINDOW_RESIZABLE |
			       SDL_WINDOW_BORDERLESS |
			       (config.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

	SDL_GetDisplayBounds(0, &fullscreen_dimensions);
	setAspectRatio();

	int bpp;
	SDL_DisplayMode mode;
	Uint32 Rmask, Gmask, Bmask, Amask;

	SDL_GetDesktopDisplayMode(0, &mode);
	SDL_PixelFormatEnumToMasks(mode.format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);

	SDL_Log("Current mode: %dx%d@%dHz, %d bits-per-pixel (%s)", mode.w, mode.h, mode.refresh_rate, bpp,
	   SDL_GetPixelFormatName(mode.format));


	window = SDL_CreateWindow("SPACE", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, W, H, flags);

	if (!window) {
		SDL_Log("ERROR - could not create window!\n");
		SDL_Quit();
		exit(-1);
	}

	SDL_ShowWindow(window);
}

static void initGL() {

	// Create an OpenGL context associated with the window.
	glcontext = SDL_GL_CreateContext(window);

	if (!glcontext) {
		SDL_Log("SDL ERROR: %s", SDL_GetError());
	}

	SDL_GL_SetSwapInterval(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    /* set the clear color to dark blue */
//	glClearColor(0, 0.08, 0.15, 1);

	/* print gl info */
	SDL_Log("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	SDL_Log("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	SDL_Log("GL_VERSION: %s\n", glGetString(GL_VERSION));
	////SDL_Log("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glMatrixMode(GL_PROJECTION);
	draw_load_identity();
	GLfloat W_2 = WIDTH / 2;
	GLfloat H_2 = HEIGHT / 2;

	glOrtho(-W_2, W_2, -H_2, H_2, 1, -1);

	glMatrixMode(GL_MODELVIEW);
	draw_load_identity();

	glViewport(0, 0, W, H);
	glClearColor(0,0,0, 1);

	glEnableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_MULTISAMPLE);
	glDisable(GL_DEPTH_TEST);

	SDL_Log("DEBUG - initGL done!\n");
}

static void main_init() {
	srand(time(0)); /* init pseudo-random generator */

	//TODO Make sure faulty inits stops the program from proceeding
	init_config();      /* load default and user-changed settings */

	display_init();     /* sets attributes and creates windows and renderer*/
	initGL();           /* setup a gl context */

	waffle_init();      /* prepare game resources and general methods*/
	texture_init();     /* preload textures */
	draw_init();        /* initializes circular shapes and rainbow colors */
	level_init();       /* load levels */
	particles_init();   /* load and prepare all particle systems */
	//font_init();      /* (currently not in use) */
	statesystem_init(); /* init all states */

	statesystem_set_state(STATESYSTEM_MENU);
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



		deltaTime = deltaTime > 0.25 ? 0.25 : deltaTime;
		dt = deltaTime;
		mdt = dt * 1000;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		statesystem_update();
		draw_load_identity();
		statesystem_draw();


        frames += deltaTime;
        fps++;
        if (frames >= 1) {
            sprintf(fps_buf, "%.2f FPS", fps);
            SDL_Log("%s\n", fps_buf);
            frames = 0;
            fps = 0;
            fprintf(stderr,"Frametime: %d \n",SDL_GetTicks() - thisTime);
        }


		int gl_error = glGetError();
		if (gl_error) SDL_Log("main.c: %d  GL_ERROR: %d\n",__LINE__,gl_error);

		SDL_GL_SwapWindow(window);

		//input handler
		if (keys[SDL_SCANCODE_F12]) {
			main_stop();
		}

		if (keys[SDL_SCANCODE_F] && keys[SDL_SCANCODE_LCTRL]) {
			config.fullscreen ^= 1; // Toggle fullscreen
			if (SDL_SetWindowFullscreen(window, config.fullscreen)) {
				SDL_Log(SDL_GetError());
				config.fullscreen ^= 1; // Re-toggle
			} else {
				glViewport(0, 0, W, H);
				setAspectRatio();
			}
			keys[SDL_SCANCODE_F] = 0;
		}

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				main_stop();
				break;
			case SDL_WINDOWEVENT_RESIZED:
				if (event.window.windowID == SDL_GetWindowID(window)) {
					W = event.window.data1;
					H = event.window.data2;

					int status = SDL_GL_MakeCurrent(window,glcontext);
					if (status) {
						SDL_Log("SDL_GL_MakeCurrent(): %s", SDL_GetError());
					}

					glViewport(0, 0, W, H);

					setAspectRatio();
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				MOUSE_X_PRESSED = event.button.x;
				MOUSE_Y_PRESSED = event.button.y;
				// very tmp touch controller
				keys[KEY_RETURN_1] = 1;
				keys[KEY_RETURN_2] = 1;
				keys[KEY_RIGHT_1] = 1;
				keys[KEY_UP_1] = 1;
				keys[KEY_RIGHT_2] = 1;
				break;
			case SDL_MOUSEBUTTONUP:
				// very tmp touch controller
				keys[KEY_RETURN_1] = 0;
				keys[KEY_RETURN_2] = 0;
				keys[KEY_RIGHT_1] = 0;
				keys[KEY_UP_1] = 0;
				keys[KEY_RIGHT_2] = 0;
				break;
			case SDL_MOUSEMOTION:
				MOUSE_X = event.button.x;
				MOUSE_Y = event.button.y;
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

	/*TODO idea:
	 * add stack with callback functions to both destroy- and init methods of modules
	 * and then loop through stack to call the functions automatically
	 */

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	//SDL_GL_MakeCurrent(NULL, NULL);
	SDL_GL_DeleteContext(glcontext);
	SDL_VideoQuit();
	SDL_Quit();

	return 0;
}

int main(int argc, char *args[]) {
	main_init();
	main_run();
	main_destroy();

	exit(0);
	return 0;
}

int main_stop() {
	main_running = 0;

	return main_running;
}
