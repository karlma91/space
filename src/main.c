#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"

#include "space.h"

static void game_destroy();

static float fps;
static float frames;
static int i,j;

//external
char fps_buf[15];
int WIDTH;
int HEIGHT;
Uint8 *keys;
struct state *currentState;


void initGL()
{
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
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
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_COLOR_MATERIAL);
	
	float aspect = (float)WIDTH / (float)HEIGHT;
	
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
	
	//store punkter
	glPointSize(8.0f);
	
	/* We want z-buffer tests enabled*/
	//glEnable(GL_DEPTH_TEST);
	//enables gldraw array
	glEnableClientState(GL_VERTEX_ARRAY);
	
	/* Do draw back-facing polygons*/
	glDisable(GL_CULL_FACE);
}

int main( int argc, char* args[] )
{
	SDL_Surface *screen;
	SDL_Event event;
	
	Uint32 thisTime = 0; 
	Uint32 lastTime;
	float deltaTime = 0.0f;
	
	int keypress = 0;
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
	
	//random seed
	
	const SDL_VideoInfo* myPointer = SDL_GetVideoInfo();
	WIDTH = myPointer->current_w;
	HEIGHT = myPointer->current_h;
	
	if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_OPENGL | SDL_FULLSCREEN)))
	{
		printf("ERROR");
		SDL_Quit();
		return 1;
	}
	
	initGL();
	font_init();
	SPACE_init();
	draw_init();
	
	currentState = &spaceState;
	
	lastTime = SDL_GetTicks();
	while(!keypress) 
	{
		
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
		
		//Draw
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		
		currentState->render(deltaTime);
		currentState->update(deltaTime);
		SDL_GL_SwapBuffers();
		
		if(keys[SDLK_ESCAPE]){
			keypress = 1;
		}
		
		while(SDL_PollEvent(&event)) 
		{      
			switch (event.type) 
			{
				case SDL_QUIT:
					keypress = 1;
					break;
			}
		}
		
		//not use 100% of cpu
		SDL_Delay(SLEEP_TIME);
	}
	game_destroy();
	SDL_Quit();
	return 0;
}

static void game_destroy()
{
	//cpSpaceFreeChildren(space);
	SPACE_destroy();
	draw_destroy();
	font_destroy();
}
