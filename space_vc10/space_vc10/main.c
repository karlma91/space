#include "SDL.h"
#include "chipmunk/chipmunk.h"
#include "SDL_opengl.h"

#define WIDTH  640
#define HEIGHT 480

void drawSpace(cpSpace *space);
void drawShape(cpShape *shape, void *unused);

cpSpace *space;
Uint8 *keys;
cpBody *player;
float frames = 0;

void draw(float dt)
{
	frames+=dt;
	if(frames>=1){
		printf("%.2f FPS\n",1/dt);
		frames = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	glTranslatef(-player->p.x, -player->p.y, 0.0f);

	//cpVect pos = cpBodyGetPos(player);
    //cpVect vel = cpBodyGetVel(ballBody);
	 cpBodySetForce(player, cpv(0,0));
	if(keys[SDLK_w]){
			cpBodySetForce(player, cpv(0,10000));
	 } else if(keys[SDLK_a]){
			cpBodySetForce(player, cpv(-10000,0));
	 } else if(keys[SDLK_s]){
			cpBodySetForce(player, cpv(0,-10000));
	 } else if(keys[SDLK_d]){
			cpBodySetForce(player, cpv(10000,0));
	 }

    //cpBodyApplyImpulse(player, cpv(0, 50), cpv(0, 0));
	drawSpace(space);

	SDL_GL_SwapBuffers();

	cpFloat cpdt = dt;
	
	cpSpaceStep(space, cpdt);
}

void drawSpace(cpSpace *space)
{
	glBegin( GL_POINTS );
	glColor3f( 0.95f, 0.207f, 0.031f );
	cpSpaceEachShape(space, drawShape, NULL);
	glEnd();
}

void drawShape(cpShape *shape, void *unused)
{
	cpCircleShape *circle = (cpCircleShape *)shape;
	glVertex2f(circle->tc.x, circle->tc.y);
}

void initBall(){
  cpVect gravity = cpv(0, -100);
  
  space = cpSpaceNew();
  cpSpaceSetGravity(space, gravity);
 
  cpShape *ground = cpSegmentShapeNew(space->staticBody, cpv(0, -300), cpv(WIDTH, -HEIGHT+100), 0);
  cpShapeSetFriction(ground, 1);
  cpSpaceAddShape(space, ground);
  
  ground = cpSegmentShapeNew(space->staticBody, cpv(400, 400), cpv(400, -400), 0);
  cpShapeSetFriction(ground, 1);
  cpSpaceAddShape(space, ground);
  
  ground = cpSegmentShapeNew(space->staticBody, cpv(0, -300), cpv(0, 300), 0);
  cpShapeSetFriction(ground, 1);
  cpSpaceAddShape(space, ground);
  
  cpFloat radius = 10;
  cpFloat mass = 1;
  
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  //player
   player = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForCircle(10, 0, 15, cpvzero)));
		  cpBodySetPos(player, cpv(15*20, -1*30));
		  
		  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(player, 15, cpvzero));
		  cpShapeSetFriction(ballShape, 0.7);
  
  for(int i = 1; i<10; i++){
	  for(int j = 1; j<15; j++){
		  cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
		  cpBodySetPos(ballBody, cpv(j*20, i*30));
		  
		  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
		  cpShapeSetFriction(ballShape, 0.7);
	  }
  }
}

void init(){
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

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
    glEnable(GL_DEPTH_TEST);

    /* Do draw back-facing polygons*/
    glDisable(GL_CULL_FACE);
	initBall();
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
   
   //SDL_FULLSCREEN|
    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_OPENGL)))
    {
		printf("ERROR");
        SDL_Quit();
        return 1;
    }
	init();
	lastTime = SDL_GetTicks();
    while(!keypress) 
    {
	
		thisTime = SDL_GetTicks();
		deltaTime = (float)(thisTime - lastTime)/1000.0f;
		lastTime = thisTime;
	
		SDL_PumpEvents();
		keys = SDL_GetKeyState(NULL);
		
         draw(deltaTime);
		 
		 if(keys[SDLK_q]){
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
		 SDL_Delay(1);
    }
	
    SDL_Quit();
    return 0;
}