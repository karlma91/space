#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"


void drawStars();
void drawSpace(cpSpace *space);
void drawShape(cpShape *shape, void *unused);
void game_destroy();

int WIDTH;
int HEIGHT;
float accumulator;

TTF_Font *font;

cpSpace *space;
Uint8 *keys;
cpBody *player;
float frames = 0;
cpFloat phys_step = 1/60.0f;

static int i,j;

//planet stuff
static cpBody *planetBody;
static cpFloat gravityStrength = 5.0e11f;

//camera settings
static int cam_relative = 0;
static float cam_zoom = 1.0f;

#define star_count 10
int stars_x[star_count];
int stars_y[star_count];

int planet_size = 50000;

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
  rot = cpvmult(rot, 2500);
  cpBodySetForce(player, cpv(0,0));
  cpBodySetTorque(player, 0);
  
  if(keys[SDLK_w]) cpBodySetForce(player, rot);
  if(keys[SDLK_s]) cpBodySetForce(player, cpvneg(rot));
  if(keys[SDLK_d]) cpBodySetTorque(player, -750);
  if(keys[SDLK_a]) cpBodySetTorque(player, 750);
  
  if (keys[SDLK_SPACE]) {
    cpBodySetVelLimit(player,5000);
    cpBodySetAngVelLimit(player,2);
    cpBodySetVel(player, cpvzero);
    cpBodySetAngVel(player, 0);
  }
  
  static int F1_pushed = 0;
  if(keys[SDLK_F1]){
    if (!F1_pushed) {
      F1_pushed = 1;
      cam_relative = !cam_relative;
    }
  } else F1_pushed = 0;
  
  if (keys[SDLK_1]){
    cam_zoom /= dt+1.2f;
  }
  if (keys[SDLK_2]){
    cam_zoom *= dt+1.2f;
    if (keys[SDLK_1])
      cam_zoom = 1;  
  }
  
  while(accumulator >= phys_step)
    {
      cpSpaceStep(space, phys_step);
      accumulator -= phys_step;
    }
  
  //Draw
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  
  //camera zoom
  glScalef(cam_zoom,cam_zoom,cam_zoom);
  static float r;
  r = atan2(player->p.y,player->p.x);
  if (cam_relative) 
    glRotatef(-cpBodyGetAngle(player) * 180/3.14f,0,0,1);
  else {
    glRotatef(-r * 180/3.14f  + 90,0,0,1);
  }
  glTranslatef(-player->p.x, -player->p.y, 0.0f);
  drawStars();
  drawSpace(space);
  glLoadIdentity();
  RenderText(font, RGBAColor(0.95f, 1.0f, 1.0f,1.0f),-100, 100,"UBERFONT");
  
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
  void (*functionPtr)(cpShape*);
  functionPtr  = cpShapeGetUserData(shape);
  if(functionPtr != NULL){
	(*functionPtr)(shape);
  }
}
void drawBall(cpShape *shape){
	cpCircleShape *circle = (cpCircleShape *)shape;
	drawCircle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), 10,cam_zoom, RGBAColor(0.80f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
}

void drawPlanet(cpShape *shape){
	cpCircleShape *circle = (cpCircleShape *)shape;
	drawCircle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), planet_size,cam_zoom, RGBAColor(0.95f, 0.207f, 0.05f,1.0f), RGBAColor(1.0f, 1.0f, 1.0f,0.0f));
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

void player_draw(cpShape *shape){
	cpCircleShape *circle = (cpCircleShape *)shape;
	drawCircle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), 15,cam_zoom, RGBAColor(0.95f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
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
  cpShapeSetUserData(ballShape, player_draw);
  
  for(i = 1; i<10; i++){
    for(j = 1; j<10; j++){
      cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
      //planet gravity thing
      ballBody->velocity_func = planetGravityVelocityFunc;
      cpBodySetPos(ballBody, cpv(j*20, planet_size + 10+i*30));
      // cpBodySetVel(ballBody, cpv(600,0));
		  
      cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
      cpShapeSetFriction(ballShape, 0.7);
	  cpShapeSetUserData(ballShape, drawBall);
    }

  }
  //planet stuff
  planetBody = cpBodyNew(INFINITY, INFINITY);
  cpBodySetAngVel(planetBody, 0.2f);
  cpBodySetPos(planetBody, cpv(0, 0));

  cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(planetBody, planet_size, cpvzero));
  cpShapeSetElasticity(shape, 1.0f);
  cpShapeSetFriction(shape, 0.0f);
  cpShapeSetUserData(shape, drawPlanet);

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

  //antialiasing
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );  
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  
  //fra ttf opengl tutorial
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
  draw_init();
  TTF_Init();
  
  font = loadfont("fonts/JuraMedium.ttf", 32);
  
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
      SDL_Delay(2);
    }
	game_destroy();
  SDL_Quit();
  return 0;
}

void game_destroy(){
  //cpSpaceFreeChildren(space);
  cpSpaceDestroy(space);
  draw_destroy();
}
