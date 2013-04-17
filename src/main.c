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

int W,H;

float dt;
float mdt;
unsigned char *keys;
state *currentState;

static int main_running = 1;

configuration config;

static int handler(void* config, const char* section, const char* name, const char* value)
{
  configuration* pconfig = (configuration*)config;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if (MATCH("video", "fullscreen")) {
	  pconfig->fullscreen = atoi(value);
  }else if (MATCH("video", "arcade")) {
	  pconfig->arcade = atoi(value);
  } else if (MATCH("video", "width")) {
	  pconfig->width = atoi(value);
  } else if (MATCH("video", "height")) {
	  pconfig->height= atoi(value);
  } else if (MATCH("keyboard", "key_left")) {
	  pconfig->key_left = atoi(value);
  } else if (MATCH("keyboard", "key_up")) {
  	pconfig->key_up = atoi(value);
  } else if (MATCH("keyboard", "key_right")) {
  	pconfig->key_right = atoi(value);
  } else if (MATCH("keyboard", "key_down")) {
  	pconfig->key_down = atoi(value);
  } else {
      return 0;  /* unknown section/name, error */
  }
  return 1;
}

static int init_config()
{
	if (ini_parse("bin/config.ini", handler, &config) < 0) {
		printf("Could not load 'bin/config.ini'\n");
		return 1;
	}
	//fprintf(stderr,"Config loaded from 'bin/config.ini': fullscreen=%d\n", config.fullscreen);
	return 0;
}

static void initGL()
{
	//antialiasing
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POLYGON_SMOOTH);
	 glEnable(GL_NEAREST);
	 glDisable(GL_POLYGON_SMOOTH);
	 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//fra ttf opengl tutorial
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	/* Make the viewport cover the whole window */
	glViewport(0, 0, W, H);

	/* Set the camera projection matrix:
	 * field of view: 90 degrees
	 * near clipping plane at 0.1
	 * far clipping plane at 100.0
	 */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-(WIDTH/2),(WIDTH/2),-(HEIGHT/2),(HEIGHT/2),1,-1);

	//gluPerspective(60.0, aspect, 0.1, 100.0);
	/* We're done with the camera, now matrix operations
	 * will affect the modelview matrix
	 * */
	glMatrixMode(GL_MODELVIEW);

	/* set the clear color to gray */
	//glClearColor(0,0.08,0.15, 1);
	glClearColor(0,0,0, 1);

	//enables gldraw array
	glEnableClientState(GL_VERTEX_ARRAY);

	/* Do draw back-facing polygons*/
	glDisable(GL_CULL_FACE);
}

SDL_Surface *screen;

void setAspectRatio() {
	WIDTH = 1920;
	HEIGHT = (1.0f * H/W) * WIDTH;
}

static int main_init()
{
	init_config();

	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;

	if (config.fullscreen) {
		const SDL_VideoInfo* myPointer = SDL_GetVideoInfo();
		W = myPointer->current_w;
		H = myPointer->current_h;
	} else {
		W = config.width;
		H = config.height;
	}

	setAspectRatio();

	//glOrtho(-(WIDTH/2),(WIDTH/2),-(HEIGHT/2),(HEIGHT/2),1,-1);

	/* NB: need to be set before call to SDL_SetVideoMode! */
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	//for antialiasing
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); //TODO fix tile edges when AA is activated
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2); //TODO read AA-settings from config file

	SDL_putenv("SDL_DEBUG");

	SDL_putenv("SDL_VIDEO_CENTERED=center");
	SDL_WM_SetCaption("SPACE", "SPACE");

	{
		SDL_Surface     *image;
#ifdef __WIN32__
		image = SDL_LoadBMP("textures/icon_32.bmp"); // windows is only compatible with 32x32 icon
#else
		image = SDL_LoadBMP("textures/icon.bmp");
#endif
		SDL_WM_SetIcon(image, NULL); //TODO fix transparency
		//Uint32          colorkey;
		//colorkey = SDL_MapRGB(image->format, 255, 0, 0);
		//SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);
	}

	if (!(screen = SDL_SetVideoMode(W, H, 0, (SDL_OPENGL| SDL_DOUBLEBUF | SDL_RESIZABLE) | (SDL_FULLSCREEN * config.fullscreen))))
	{
		printf("ERROR");
		SDL_Quit();
		return 1;
	}
	// random seed
	srand(time( NULL ));

	int error;

	initGL();

	/* preload textures */
	int color_type = screen->format->Bshift ? GL_UNSIGNED_INT_8_8_8_8 : GL_UNSIGNED_INT_8_8_8_8_REV;
	texture_init(color_type);

	error = draw_init();
	if(error){
		return error;
	}

	font_init();
	particles_init();

	/* load levels */
	error = level_init();
	if(error){
		return error;
	}

	/* init states */
	statesystem_init();
	statesystem_set_state(STATESYSTEM_MENU);

	return 0;
}

static int main_run()
{
  SDL_Event event;
	Uint32 thisTime = 0;
	Uint32 lastTime;
	float deltaTime = 0.0f;

	statesystem_set_state(STATESYSTEM_MENU);

	lastTime = SDL_GetTicks();

	//START GAME
	if(config.arcade){
		printf("start %s\n", "999");
	}

	while(main_running) {

		thisTime = SDL_GetTicks();
		deltaTime = (float)(thisTime - lastTime)/1000.0f;
		lastTime = thisTime;

		SDL_PumpEvents();
		keys = SDL_GetKeyState(NULL);

		frames += deltaTime;
		fps++;
		if(frames>=1){
			sprintf(fps_buf,"%.2f FPS",fps);
			//printf("%s\n",fps_buf);
			frames = 0;
			fps=0;
		}

		deltaTime = deltaTime > 0.25 ? 0.25 : deltaTime;
		dt = deltaTime;
		mdt = dt*1000;
		//Draw
		//glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		statesystem_update();
		statesystem_draw();

		SDL_GL_SwapBuffers();

		//input handler
		if(keys[SDLK_F12]){
			main_stop();
		}

		if (keys[SDLK_f] && keys[SDLK_LCTRL]) {
			int ret = SDL_WM_ToggleFullScreen(screen);
			fprintf(stderr,"TOGGLING FULLSCREEN SUCCESS: %d\n",ret);
			keys[SDLK_f] = 0;
		}

		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
			    main_stop();
				break;
			case SDL_VIDEORESIZE:
				if (config.fullscreen) break;
				W = event.resize.w;
				H = event.resize.h;
				if (!(screen = SDL_SetVideoMode(W, H, 32, (SDL_OPENGL| SDL_DOUBLEBUF | SDL_RESIZABLE) | (SDL_FULLSCREEN * config.fullscreen))))
				{
					printf("ERROR");
					SDL_Quit();
					return 1;
				}
				glViewport(0,0,W,H);
				setAspectRatio();
				break;
			}
		}

		//not use 100% of cpu
		if (keys[SDLK_z])
			 SDL_Delay(14);
		else if (keys[SDLK_c])
			SDL_Delay(50);

		static int FPS_EXTRA_SLEEP = 0;
		if (FPS_EXTRA_SLEEP >= 2)
			FPS_EXTRA_SLEEP = 0;
		else
			++FPS_EXTRA_SLEEP;


		Uint32 sleep_delta = SDL_GetTicks() - thisTime;
		if(  sleep_delta < FPS_SLEEP_TIME ) {
			SDL_Delay( ( FPS_SLEEP_TIME ) - sleep_delta + (FPS_EXTRA_SLEEP != 0));
		}
	}
	return 0;
}

static int main_destroy()
{
	level_destroy();
	//cpSpaceFreeChildren(space);

	/* destroy states */
	statesystem_destroy();

	draw_destroy();
	font_destroy();

	SDL_Quit();

	return 0;
}

int main( int argc, char *args[] )
{
    main_init();
	main_run();
	main_destroy();

	return 0;
}

int main_stop() {
	main_running = 0;

	return main_running;
}
