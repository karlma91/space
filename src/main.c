#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"

void drawStars();
void drawSpace(cpSpace *space);
void drawShape(cpShape *shape, void *unused);

int WIDTH;
int HEIGHT;
float accumulator;

cpSpace *space;
Uint8 *keys;
cpBody *player;
float frames = 0;
cpFloat phys_step = 1/60.0f;

static int i,j;

#define star_count 10000
int stars_x[star_count];
int stars_y[star_count];

void draw(float dt) 
{
  dt = dt > 0.25 ? 0.25 : dt;
  accumulator += dt;
  frames += dt;
  if(frames>=1){
    printf("%.2f FPS\n",1/dt);
    frames = 0;
  }
  
  //update physics and player
  cpVect rot = cpBodyGetRot(player);
  rot = cpvmult(rot, 1000);
  cpBodySetForce(player, cpv(0,0));
  cpBodySetTorque(player, 0);
  if(keys[SDLK_w]) {
    cpBodySetForce(player, rot);
  }
  if(keys[SDLK_a]){
    cpBodySetTorque(player, 1000);
  }
  if(keys[SDLK_s]){
    cpBodySetForce(player, cpvneg(rot));
  }
  if(keys[SDLK_d]){
    cpBodySetTorque(player, -1000);
  }
  if (keys[SDLK_SPACE]) {
    cpBodySetVelLimit(player,1000);
    cpBodySetAngVelLimit(player,10);
    cpBodySetVel(player, cpvzero);
    cpBodySetAngVel(player, 0);
  }
  
  while(accumulator >= phys_step)
    {
      cpSpaceStep(space, phys_step);
      accumulator -= phys_step;
    }
  
  //Draw
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glRotatef(-cpBodyGetAngle(player) * 180/3.14f,0,0,1);
  glTranslatef(-player->p.x, -player->p.y, 0.0f);

  drawSpace(space);
  drawStars();
  SDL_GL_SwapBuffers();
}

void drawSpace(cpSpace *space)
{
  cpSpaceEachShape(space, drawShape, NULL);
}

#define SW (1920)
void drawStars()
{
  glPointSize(2.0f);
  glColor3f(1,1,1);

  glPushMatrix();
  glTranslatef(((int)player->p.x+SW/2) / SW * SW,((int)player->p.y+SW/2) / SW * SW,0);

  glBegin(GL_POINTS);
  for (i=0;i<star_count;i++) {
    glVertex2f(stars_x[i],stars_y[i]);
  }
  glEnd();

  glPopMatrix();
}

float x,y,r;

void drawShape(cpShape *shape, void *unused)
{
  cpCircleShape *circle = (cpCircleShape *)shape;
  cpFloat angle = cpBodyGetAngle(cpShapeGetBody(shape)) * 180/3.14f;
  x = circle->tc.x;
  y = circle->tc.y;
  r = 10;

  glPushMatrix();
  glTranslatef(x,y,0);
  glRotatef(angle,0,0,1);

  glBegin(GL_TRIANGLES);
  {
    glColor3f( 0.95f, 0.207f, 0.031f );
    glVertex2f(-r,-r);

    glColor3f( 0.05f, 0.207f, 1.0f );
    glVertex2f(-r,r);

    glColor3f( 0.95f, 1.0f, 0.6f );
    glVertex2f(r*2,0);
  }
  glEnd();

  glPopMatrix();
}

void initBall(){
  cpVect gravity = cpv(0, -0);
  
  space = cpSpaceNew();
  cpSpaceSetGravity(space, gravity);
  //init stars
  srand(122531);
  for (i=0;i<star_count;i++) {
    stars_x[i] = rand()%(WIDTH*2) - WIDTH;
    stars_y[i] = rand()%(WIDTH*2) - WIDTH;
  }

  /*
  cpShape *ground = cpSegmentShapeNew(space->staticBody, cpv(0, -300), cpv(WIDTH, -HEIGHT+100), 0);
  cpShapeSetFriction(ground, 1);
  cpSpaceAddShape(space, ground);
  
  ground = cpSegmentShapeNew(space->staticBody, cpv(400, 400), cpv(400, -400), 0);
  cpShapeSetFriction(ground, 1);
  cpSpaceAddShape(space, ground);
  
  ground = cpSegmentShapeNew(space->staticBody, cpv(0, -300), cpv(0, 300), 0);
  cpShapeSetFriction(ground, 1);
  cpSpaceAddShape(space, ground);
  */
  cpFloat radius = 10;
  cpFloat mass = 1;
  
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  //player
  player = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForCircle(10, 0, 15, cpvzero)));
  cpBodySetPos(player, cpv(15*20, -1*30));
		  
  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(player, 15, cpvzero));
  cpShapeSetFriction(ballShape, 0.7);
  
  for(i = 1; i<10; i++){
    for(j = 1; j<15; j++){
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


  const SDL_VideoInfo* myPointer = SDL_GetVideoInfo();
  WIDTH = myPointer->current_w;
  HEIGHT = myPointer->current_h;

   
  //SDL_FULLSCREEN|
  if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_OPENGL | SDL_FULLSCREEN)))
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
