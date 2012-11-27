#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"

void drawStars();
void drawSpace(cpSpace *space);
void drawShape(cpShape *shape, void *unused);
void drawCircle(GLfloat *array,int len, cpVect center, cpFloat angle, cpFloat radius, int line);

int WIDTH;
int HEIGHT;
float accumulator;

cpSpace *space;
Uint8 *keys;
cpBody *player;
float frames = 0;
cpFloat phys_step = 1/60.0f;

static int i,j;

//planet stuff
static cpBody *planetBody;
static cpFloat gravityStrength = 5.0e9f;

#define star_count 10000
int stars_x[star_count];
int stars_y[star_count];

int planet_size = 10000;


static const GLfloat circleVAR[] = {
	 0.0000f,  1.0000f,
	 0.2588f,  0.9659f,
	 0.5000f,  0.8660f,
	 0.7071f,  0.7071f,
	 0.8660f,  0.5000f,
	 0.9659f,  0.2588f,
	 1.0000f,  0.0000f,
	 0.9659f, -0.2588f,
	 0.8660f, -0.5000f,
	 0.7071f, -0.7071f,
	 0.5000f, -0.8660f,
	 0.2588f, -0.9659f,
	 0.0000f, -1.0000f,
	-0.2588f, -0.9659f,
	-0.5000f, -0.8660f,
	-0.7071f, -0.7071f,
	-0.8660f, -0.5000f,
	-0.9659f, -0.2588f,
	-1.0000f, -0.0000f,
	-0.9659f,  0.2588f,
	-0.8660f,  0.5000f,
	-0.7071f,  0.7071f,
	-0.5000f,  0.8660f,
	-0.2588f,  0.9659f,
	 0.0000f,  1.0000f,
	 0.0f, 0.0f, // For an extra line to see the rotation.
};

#define circleP_count 1000
static GLfloat circleP[circleP_count];

static const int circleVAR_count = sizeof(circleVAR)/sizeof(GLfloat)/2;

float x,y,r;


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
  rot = cpvmult(rot, 3000);
  cpBodySetForce(player, cpv(0,0));
  cpBodySetTorque(player, 0);
  if(keys[SDLK_w]) {
    cpBodySetForce(player, rot);
  }
  if(keys[SDLK_a]){
    cpBodySetTorque(player, 2000);
  }
  if(keys[SDLK_s]){
    cpBodySetForce(player, cpvneg(rot));
  }
  if(keys[SDLK_d]){
    cpBodySetTorque(player, -2000);
  }
  if (keys[SDLK_SPACE]) {
    //cpBodySetVelLimit(player,1000);
    cpBodySetAngVelLimit(player,5);
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
  drawStars();
  drawSpace(space);

  drawCircle(circleP,circleP_count,planetBody->p,cpBodyGetAngle(planetBody),planet_size,0);

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


void drawShape(cpShape *shape, void *unused)
{
  cpCircleShape *circle = (cpCircleShape *)shape;
  r = 10;
  drawCircle(circleVAR, circleVAR_count, circle->tc,cpBodyGetAngle(cpShapeGetBody(shape)),r,1);

}

static void
planetGravityVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	// Gravitational acceleration is proportional to the inverse square of
	// distance, and directed toward the origin. The central planet is assumed
	// to be massive enough that it affects the satellites but not vice versa.
	cpVect p = cpBodyGetPos(body);
	cpFloat sqdist = cpvlengthsq(p);
	cpVect g = cpvmult(p, -gravityStrength / (sqdist * cpfsqrt(sqdist)));
	
	cpBodyUpdateVelocity(body, g, damping, dt);
}

void drawCircle(GLfloat *array,int len, cpVect center, cpFloat angle, cpFloat radius, int line)
{
  glVertexPointer(2, GL_FLOAT, 0, array);

  glPushMatrix(); {
    glTranslatef(center.x, center.y, 0.0f);
    glRotatef(angle*180.0f/M_PI - 90, 0.0f, 0.0f, 1.0f);
    glScalef(radius, radius, 1.0f);
    
    glColor3f( 0.95f, 0.207f, 0.031f );
    glDrawArrays(GL_TRIANGLE_FAN, 0, len - 1);
    
    if (line) {
    glColor3f( 1.0f, 1.0f, 1.0f );
    glDrawArrays(GL_LINE_STRIP, 0, len);
    }
    
    } glPopMatrix();
}

void initBall(){

  
  for(i = 0; i<circleP_count-1; i++){
    circleP[i] = sin(2*M_PI*i/circleP_count);
    circleP[i+1] = cos(2*M_PI*i/circleP_count);
    i++;
  }


  cpVect gravity = cpv(0, -0);
  
  space = cpSpaceNew();
  cpSpaceSetGravity(space, gravity);
  //init stars
  srand(122531);
  for (i=0;i<star_count;i++) {
    stars_x[i] = rand()%(WIDTH*2) - WIDTH;
    stars_y[i] = rand()%(WIDTH*2) - WIDTH;
  }
  cpFloat radius = 10;
  cpFloat mass = 1;
  
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  //player
  player = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForCircle(2, 0, 15, cpvzero)));
  cpBodySetPos(player, cpv(15*20, planet_size+10+1*30));

  player->velocity_func = planetGravityVelocityFunc;  
  // cpBodySetVel(player, cpv(600,0));

  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(player, 15, cpvzero));
  cpShapeSetFriction(ballShape, 0.7);
  
  for(i = 1; i<10; i++){
    for(j = 1; j<10; j++){
      cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
      //planet gravity thing
      ballBody->velocity_func = planetGravityVelocityFunc;
      cpBodySetPos(ballBody, cpv(j*20, planet_size + 10+i*30));
      // cpBodySetVel(ballBody, cpv(600,0));
		  
      cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
      cpShapeSetFriction(ballShape, 0.7);
    }

  }
  //planet stuff
  planetBody = cpBodyNew(INFINITY, INFINITY);
  cpBodySetAngVel(planetBody, 0.2f);
  cpBodySetPos(planetBody, cpv(0, 0));

  cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(planetBody, planet_size, cpvzero));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 0.0f);

}

void init(){
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  //for antialiasing
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
  
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

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
