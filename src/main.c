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

static float fps;
static float frames;

/* definition of external variables */
char fps_buf[15];
int WIDTH;
int HEIGHT;
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
	fprintf(stderr,"Config loaded from 'bin/config.ini': fullscreen=%d",
			config.fullscreen);
	return 0;
}

static void initGL()
{
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	//for antialiasing
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	//antialiasing
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	//fra ttf opengl tutorial
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	/* Make the viewport cover the whole window */
	glViewport(0, 0, WIDTH, HEIGHT);

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
	glClearColor(0.0, 0.0 ,0.0, 0);

	//enables gldraw array
	glEnableClientState(GL_VERTEX_ARRAY);

	/* Do draw back-facing polygons*/
	glDisable(GL_CULL_FACE);
}

static int main_init()
{
	init_config();

	SDL_Surface *screen;

	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;

	const SDL_VideoInfo* myPointer = SDL_GetVideoInfo();
	WIDTH = myPointer->current_w;
	HEIGHT = myPointer->current_h;

	if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, (SDL_OPENGL| SDL_DOUBLEBUF) | (SDL_FULLSCREEN * config.fullscreen))))
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
	texture_init();

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
	state_space.init();
	state_menu.init();
	state_levelselect.init();
	state_gameover.init();

	return 0;
}

static int main_run()
{
  SDL_Event event;
	Uint32 thisTime = 0;
	Uint32 lastTime;
	float deltaTime = 0.0f;

	currentState = &state_menu;
	lastTime = SDL_GetTicks();
	int numstat = 0;
	state states[5];
	state *temps;
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
			printf("%s\n",fps_buf);
			frames = 0;
			fps=0;
		}

		deltaTime = deltaTime > 0.25 ? 0.25 : deltaTime;
		dt = deltaTime;
		mdt = dt*1000;
		//Draw
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		temps = currentState->parentState;
		while(temps){
			states[numstat] = *temps;
			temps = temps->parentState;
			numstat++;
		}
		int i = 0;
		for(i = numstat-1; i>=0; i--){
			states[i].render(deltaTime);
			glLoadIdentity();
		}

		numstat = 0;
		currentState->render(deltaTime);
		currentState->update(deltaTime);

		SDL_GL_SwapBuffers();

		//input handler
		if(keys[SDLK_F12]){
			main_stop();
		}

		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
			    main_stop();
				break;
			}
		}

		//not use 100% of cpu
		SDL_Delay(SLEEP_TIME);
	}
	return 0;
}

static int main_destroy()
{
	level_destroy();
	//cpSpaceFreeChildren(space);

	/* destroy states */
	state_space.destroy();
	state_menu.destroy();
	state_levelselect.destroy();
	state_gameover.destroy();

	draw_destroy();
	font_destroy();

	SDL_Quit();

	return 0;
}

int main( int argc, char* args[] )
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
