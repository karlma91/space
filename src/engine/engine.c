/* header */
#include "engine.h"

#include "io/waffle_utils.h"

/* standard c-libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* SDL */
#include "SDL.h"
#include "SDL_main.h"

#define SOUND_DISABLED 1
#if !SOUND_DISABLED
#include "SDL_mixer.h"
#endif

/* Chipmunk physics library */
#include "chipmunk.h"

#include "state/statesystem.h"

/* drawing code */
#include "graphics/draw.h"	//opengl included in draw.h
#include "graphics/particles.h"
#include "audio/sound.h"

#include "../space/game.h" //TODO remove dependency

#define GAME_VERSION "PRE-ALPHA 8.8" //TMP placement for this define

#define FPS_LIMIT 1

#if (GLES1)
#define glOrtho glOrthof

#define FPS_SLEEP_TIME 16//33
#else
#define FPS_SLEEP_TIME 16
#endif

static Uint32 thisTime = 0;
static Uint32 lastTime;

static int paused = 0;

static float fps;
static float frames;

/* definition of external variables */
char fps_buf[15];
int GAME_WIDTH;
float ASPECT_RATIO;
int WINDOW_WIDTH, WINDOW_HEIGHT;

camera default_camera;
camera *current_camera = &default_camera;

SDL_Joystick *accelerometer;

float dt = 0;
int mdt = 0;
unsigned char *keys;
state *currentState;

static SDL_GLContext glcontext;

SDL_Window *window;
static SDL_Rect fullscreen_dimensions;

#include "input/button.h"

static int main_running = 1;

LList active_fingers;

configuration config = {
		.fullscreen = 1,
		.width = 1920,
		.height = 1200
};

static int main_destroy();

static void main_pause()
{
	statesystem_pause();
	paused = 1;
}

static void main_unpause()
{
	paused = 0;
}

static int HandleAppEvents(void *userdata, SDL_Event *event)
{
	switch (event->type)
	{
	case SDL_QUIT:
		main_stop();
		return 1;
#if TARGET_OS_IPHONE
	case SDL_APP_TERMINATING:
		/* Terminate the app.
           Shut everything down before returning from this function.
		 */
		SDL_Log("SDL_APP_TERMINATING");
		return 0;
	case SDL_APP_LOWMEMORY:
		/* You will get this when your app is paused and iOS wants more memory.
           Release as much memory as possible.
		 */
		SDL_Log("SDL_APP_LOWMEMORY");
		return 0;
	case SDL_APP_WILLENTERBACKGROUND:
		/* Prepare your app to go into the background.  Stop loops, etc.
           This gets called when the user hits the home button, or gets a call.
		 */
		main_pause();
		SDL_Log("SDL_APP_WILLENTERBACKGROUND");
		return 0;
	case SDL_APP_DIDENTERBACKGROUND:
		/* This will get called if the user accepted whatever sent your app to the background.
           If the user got a phone call and canceled it, you'll instead get an SDL_APP_DIDENTERFOREGROUND event and restart your loops.
           When you get this, you have 5 seconds to save all your state or the app will be terminated.
           Your app is NOT active at this point.
		 */
		SDL_Log("SDL_APP_DIDENTERBACKGROUND");
		return 0;
	case SDL_APP_WILLENTERFOREGROUND:
		/* This call happens when your app is coming back to the foreground.
           Restore all your state here.
		 */
		main_unpause();
		SDL_Log("SDL_APP_WILLENTERFOREGROUND");
		return 0;
	case SDL_APP_DIDENTERFOREGROUND:
		/* Restart your loops here.
           Your app is interactive and getting CPU again.
		 */
		main_unpause();
		SDL_Log("SDL_APP_DIDENTERFOREGROUND");
		return 0;
#endif
	default:
		/* No special processing, add it to the event queue */
		return 1;
	}
}

static void setAspectRatio() {
	//SDL_ShowCursor(!config.fullscreen);

	if (config.fullscreen) {
		WINDOW_WIDTH = fullscreen_dimensions.w;
		WINDOW_HEIGHT = fullscreen_dimensions.h;
		if (WINDOW_WIDTH < WINDOW_HEIGHT) {
			int t = WINDOW_HEIGHT;
			WINDOW_HEIGHT = WINDOW_WIDTH;
			WINDOW_WIDTH = t;
		}
	} else {
		WINDOW_WIDTH = config.width;
		WINDOW_HEIGHT = config.height;
	}

	ASPECT_RATIO = 1.0f * WINDOW_WIDTH / WINDOW_HEIGHT;
	GAME_WIDTH = GAME_HEIGHT * ASPECT_RATIO;
}

static void display_init()
{
	SDL_Log("DEBUG - SDL_init\n");
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		SDL_Log("ERROR: SDL_INIT_VIDEO: %s", SDL_GetError());
		exit(-1);
	}
	SDL_Log("DEBUG - SDL_init done!\n");

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING,1);

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0); //AA not supported on Android test device
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GLES1 ? 1 : 2);
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


	window = SDL_CreateWindow("SPACE", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, flags);

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

	SDL_GL_SetSwapInterval(FPS_LIMIT); // 1 = v-sync on

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	/* set the clear color to dark blue */
	//	glClearColor(0, 0.08, 0.15, 1);

	/* print gl info */
	SDL_Log("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	SDL_Log("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	SDL_Log("GL_VERSION: %s\n", glGetString(GL_VERSION));
	SDL_Log("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));


	int max_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

	SDL_Log("Max texture size = %d", max_size);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glMatrixMode(GL_PROJECTION);
	draw_load_identity();
	GLfloat W_2 = GAME_WIDTH / 2;
	GLfloat H_2 = GAME_HEIGHT / 2;

	//TODO change to something like (0,0,GW,GH)
	glOrtho(-W_2, W_2, -H_2, H_2, 1, -1);
	//glOrtho(0, GAME_WIDTH, 0, GAME_HEIGHT, 1, -1); //option 1
	//glOrtho(0, GAME_WIDTH, GAME_HEIGHT, 0, 1, -1); //option 2 (will probably match normalized touch input)

	glMatrixMode(GL_MODELVIEW);
	draw_load_identity();

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClearColor(0,0,0, 1);

	glEnableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_MULTISAMPLE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_SCISSOR_TEST);
	glScissor(0,0, WINDOW_WIDTH,WINDOW_HEIGHT); //scissor test

	SDL_Log("DEBUG - initGL done!\n");
}

static void main_init() {
	srand(time(0)); /* init pseudo-random generator */

	//TODO Make sure faulty inits stops the program from proceeding
	waffle_init();      /* prepare game resources and general methods*/

	game_config();      /* load default and user-changed settings */

	display_init();     /* sets attributes and creates windows and renderer*/
	initGL();           /* setup a gl context */

    accelerometer = SDL_JoystickOpen(0);
    if (accelerometer == NULL) {
        SDL_Log("Could not open joystick (accelerometer)");
    }

#if __WIN32__
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		SDL_Log( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		exit(-1);
	}
	if (glewGetExtension("GL_EXT_framebuffer_object"))
	{
		SDL_Log( "Have extension" );
	}else{
		SDL_Log( "Error no extension");
	}
#endif

	cpInitChipmunk();

	sound_init();
	texture_init();     /* preload textures */
	sprite_init();
    bmfont_init();
	draw_init();        /* initializes circular shapes and rainbow colors */
	particles_init();   /* load and prepare all particle systems */
	//font_init();      /* (currently not in use) */
	statesystem_init(); /* init all states */
	game_init();

	// Handle iOS app-events (pause, low-memory, terminating, etc...) and SDL_QUIT
	SDL_SetEventFilter(HandleAppEvents, NULL);

#if TARGET_OS_IPHONE
	// Initialize the Game Center for scoring and matchmaking
    //InitGameCenter(); //TODO support GameCenter
#endif

	lastTime = SDL_GetTicks();

	active_fingers = llist_create();
}

static void check_events()
{
	SDL_Event event;
	SDL_PumpEvents();
	keys = SDL_GetKeyboardState(NULL);

	while (SDL_PollEvent(&event)) {
		statesystem_push_event(&event);
#if !GOT_TOUCH
		SDL_Event sim_event;
#endif
		static int render_outside = 1; // for debug purposes
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_T:
				render_outside ^=0x5;

				glMatrixMode(GL_PROJECTION);
				draw_load_identity();
				GLfloat W_2 = GAME_WIDTH / 2 * render_outside;
				GLfloat H_2 = GAME_HEIGHT / 2 * render_outside;
				glOrtho(-W_2, W_2, -H_2, H_2, 1, -1);
				glMatrixMode(GL_MODELVIEW);
				break;
			case SDL_SCANCODE_R:
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			default:
				break;
			}
			break;
		case SDL_FINGERUP:
			llist_remove(active_fingers, (void *) event.tfinger.fingerId);
			break;
#if !GOT_TOUCH
		case SDL_MOUSEBUTTONDOWN:
			sim_event.type = SDL_FINGERDOWN;
			sim_event.tfinger.fingerId = event.button.button;
			float x = event.button.x;
			float y = event.button.y;

			sim_event.tfinger.x = x / WINDOW_WIDTH;
			sim_event.tfinger.y = y / WINDOW_HEIGHT;
			statesystem_push_event(&sim_event);
			static float last_x, last_y;
			static int pressed;
			pressed = 1;
			last_x = x;
			last_y = y;
			break;
		case SDL_MOUSEMOTION:
			if (pressed) {
			sim_event.type = SDL_FINGERMOTION;
			sim_event.tfinger.fingerId = event.button.button;
			float x = event.button.x;
			float y = event.button.y;

			sim_event.tfinger.x = x / WINDOW_WIDTH;
			sim_event.tfinger.y = y / WINDOW_HEIGHT;

			sim_event.tfinger.dx = (event.button.x - last_x) / WINDOW_WIDTH;
			sim_event.tfinger.dy = (event.button.y - last_y) / WINDOW_HEIGHT;
			last_x = x;
			last_y = y;

			statesystem_push_event(&sim_event);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			sim_event.type = SDL_FINGERUP;
			sim_event.tfinger.fingerId = event.button.button;
			sim_event.tfinger.x = (float) event.button.x / WINDOW_WIDTH;
			sim_event.tfinger.y = (float) event.button.y / WINDOW_HEIGHT;
			statesystem_push_event(&sim_event);
			pressed = 0;
			llist_remove(active_fingers, (void *) sim_event.tfinger.fingerId);
			break;
#endif
		}
	}
}

static void main_sleep()
{
#if FPS_LIMIT
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
#else
		SDL_Delay(1);
#endif
}

static void main_tick(void *data)
{
	thisTime = SDL_GetTicks();
	dt = (thisTime - lastTime) / 1000.0f;
	lastTime = thisTime;

	dt = dt > 0.1 ? 0.1 : dt;
	mdt = dt * 1000;

	frames += dt;
	fps++;

	draw_load_identity();
	draw_color4f(0,0,0,1);
	draw_box(0,0,GAME_WIDTH,GAME_HEIGHT,0,1);

	if (!paused) {
		check_events();

		statesystem_update();
		statesystem_draw();

		int gl_error = glGetError();
		if (gl_error) SDL_Log("main.c: %d  GL_ERROR: %d\n",__LINE__,gl_error);

		if (frames >= 1) {
			sprintf(&fps_buf[0], "%.2f FPS ", fps);
			SDL_Log("%s frame: %d ms", fps_buf, SDL_GetTicks() - thisTime);
			frames = 0;
			fps = 0;
		}

		draw_load_identity();
		setTextAlign(TEXT_RIGHT);
		setTextSize(20);
		float box_width = strlen(GAME_VERSION)*20*1.5+10;
		draw_color4f(0,0,0,1);
		draw_box(GAME_WIDTH/2-box_width,-GAME_HEIGHT/2,box_width,40,0,0);
		draw_color4f(0.7,0,0,1);
		font_drawText(GAME_WIDTH/2-5,-GAME_HEIGHT/2+20,GAME_VERSION);

		SDL_GL_SwapWindow(window);
	}

	//input handler
	if (keys[SDL_SCANCODE_F12]) {
		main_stop();
	}
	if (keys[SDL_SCANCODE_R]) {
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (keys[SDL_SCANCODE_F] && keys[SDL_SCANCODE_LCTRL]) {
		config.fullscreen ^= 1; // Toggle fullscreen
		if (SDL_SetWindowFullscreen(window, config.fullscreen)) {
			SDL_Log(SDL_GetError());
			config.fullscreen ^= 1; // Re-toggle
		} else {
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			setAspectRatio();

			glMatrixMode(GL_PROJECTION);
			draw_load_identity();
			GLfloat W_2 = GAME_WIDTH / 2;
			GLfloat H_2 = GAME_HEIGHT / 2;
			glOrtho(-W_2, W_2, -H_2, H_2, 1, -1);
			glMatrixMode(GL_MODELVIEW);

			SDL_Log("DEBUG: WINDOW FULLSCREEN CHANGED -> %d", config.fullscreen);
		}
		keys[SDL_SCANCODE_F] = 0;

	}

	if (!main_running) {
		main_destroy();
	}
}

static int main_run()
{
#if __IPHONEOS__
    // Initialize the Game Center for scoring and matchmaking
    //InitGameCenter(); //TODO support GameCenter

    // Set up the game to run in the window animation callback on iOS
    // so that Game Center and so forth works correctly.
	SDL_iPhoneSetAnimationCallback(window, 1, main_tick, NULL);
#else
	//START GAME
#if ARCADE_MODE
		SDL_Log("start %s\n", "999");
#endif

	while (main_running) {
		main_tick(NULL);
		main_sleep();
	}
#endif
	return 0;
}

static int main_destroy() {
	SDL_Log("DEBUG - SDL_destroy\n");
	//cpSpaceFreeChildren(space);

	game_destroy();

	/* destroy states */
	statesystem_destroy();

	sound_destroy();
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
	SDL_Quit();

	exit(0);
	return 0;
}

int main(int argc, char *args[]) {
	main_init();
	main_run();
	return 0;
}

void main_stop() {
	main_running = 0;
}

/*
 * Converts normalized (e.g. touch input) coordinates to game window coordinates
 */
void normalized2game(float *x, float *y)
{
	*x = (*x - 0.5f) * GAME_WIDTH;
	*y = (0.5f - *y) * GAME_HEIGHT;
}

