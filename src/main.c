#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"

#define SLEEP_TIME 1

/* State  struct */
struct state {
    void (*update)(float dt);
    void (*render)(float dt);
    struct state *parentState;
};

void drawStars();
void drawSpace(cpSpace *space);
void drawShape(cpShape *shape, void *unused);
void game_destroy();

void SPACE_draw(float dt);
void SPACE_update(float dt);

void GROUND_draw(float dt);
void GROUND_update(float dt);


static void planetGravityVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);
void drawBall(cpShape *shape);

/* current state in use */
static struct state *currentState = NULL;

static struct state spaceState = {
  SPACE_draw,
  SPACE_update,
  NULL
};

static struct state groundState = {
  GROUND_draw,
  GROUND_update,
  NULL
};


int WIDTH;
int HEIGHT;
float accumulator;

cpSpace *space;
Uint8 *keys;
cpBody *player;
float frames = 0;
cpFloat phys_step = 1/60.0f;

static int i,j;

//camera settings
static int cam_relative = 1;
static float cam_zoom = 3.0f;

#define star_count 1000
int stars_x[star_count];
int stars_y[star_count];
float stars_size[star_count];

float x,y,r;
float fps;
char fps_buf[15];

void tmp_shoot() {
  cpFloat radius = 10;
  cpFloat mass = 1;
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

  cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
  //planet gravity thing
  //ballBody->velocity_func = planetGravityVelocityFunc;
  cpBodySetPos(ballBody, cpv(player->p.x, player->p.y));
  cpBodySetVel(ballBody, cpvadd(cpBodyGetVel(player),cpBodyGetVel(player)));
		  
  cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
  cpShapeSetFriction(ballShape, 0.7);
  cpShapeSetUserData(ballShape, drawBall);
}

void GROUND_draw(float dt)
{
  setTextAlign(TEXT_LEFT);
  setTextSize(10);
  glColor3f(1,1,1);
  glLineWidth(2);
  glPointSize(2);
  drawText(-WIDTH/2+15,HEIGHT/2 - 10,"TESTETESTETSTE");
  int lvlWidth = 10000;
  glBegin(GL_QUADS);
  glVertex2f(-lvlWidth, -(HEIGHT/2) +100);
  glVertex2f(-lvlWidth, -(HEIGHT/2));
    glVertex2f(lvlWidth, -(HEIGHT/2));
    glVertex2f(lvlWidth, -(HEIGHT/2)+100);
  glEnd();

}

void GROUND_update(float dt)
{
  if(keys[SDLK_g]) currentState= &spaceState;
  keys[SDLK_g] = 0;
}

void SPACE_update(float dt)
{
  
  accumulator += dt;
  frames += dt;
  if(frames>=1){
		fps = 1/dt;
		sprintf(fps_buf,"%.2f FPS",fps);
    printf("%s\n",fps_buf);
    frames = 0;
  }
  
  //update physics and player
  cpVect rot = cpBodyGetRot(player);
  rot = cpvmult(rot, 5500);
  cpBodySetForce(player, cpv(0,0));
  cpBodySetTorque(player, 0);
  /*	
	if(keys[SDLK_w])
		cpBodySetAngVel(player, -2);
  else if(keys[SDLK_s])
		cpBodySetAngVel(player, 2);
	else
		cpBodySetAngVel(player, 0);

	if(keys[SDLK_d]) cpBodySetVel(player, rot);
  if(keys[SDLK_a]) cpBodySetVel(player, rot);
  */
	
  if(keys[SDLK_w]) cpBodySetForce(player, rot);
  if(keys[SDLK_s]) cpBodySetForce(player, cpvneg(rot));
  if(keys[SDLK_d]) cpBodySetTorque(player, -5000);
  if(keys[SDLK_a]) cpBodySetTorque(player, 5000);
  

  static int F1_pushed = 0;
  if(keys[SDLK_F1]){
    if (!F1_pushed) { 
      F1_pushed = 1;
      cam_relative = !cam_relative;
    }
  } else F1_pushed = 0;

  if(keys[SDLK_g]){
    currentState = &groundState;
    keys[SDLK_g] = 0;
    cpVect gravity = cpv(0, -2);
    cpSpaceSetGravity(space, gravity);
  }

  if (keys[SDLK_q]){
    cam_zoom /= dt+1.1f;
  }
  if (keys[SDLK_e]){
    cam_zoom *= dt+1.1f;
    if (keys[SDLK_q])
      cam_zoom = 1;  
  }
  
  while(accumulator >= phys_step)
    {
      cpSpaceStep(space, phys_step);
      accumulator -= phys_step;
    }

}

void SPACE_draw(float dt) 
{
  
  //camera zoom
  glScalef(cam_zoom,cam_zoom,cam_zoom);
  static float r;
  if (cam_relative){
    
  }else {
    glRotatef(-r * MATH_180PI  + 90,0,0,1);
  }
  glTranslatef(-player->p.x, -player->p.y, 0.0f);
  drawStars();
  drawSpace(space);

  glLoadIdentity();
	
  glLineWidth(10);
	
  if (keys[SDLK_h]) {
    cpBodySetVelLimit(player,5000);
    cpBodySetAngVelLimit(player,2);
    cpBodySetVel(player, cpvzero);
    cpBodySetAngVel(player, 0);
		
    glLineWidth(1);
  }
	
  if (keys[SDLK_SPACE]) {
    tmp_shoot();
  }
	
  //draw GUI
  glColor3f(cos((player->p.x/50)),sin((player->p.y/100)),player->p.x/2550.0f*player->p.y/2550.0f);
	
  glPointSize(10);
  setTextAngle(0);
  setTextSize(80);
  setTextAlign(TEXT_CENTER);
  drawText(0,0.8f*HEIGHT/2, "SPACE");
	
  setTextAlign(TEXT_LEFT);
  setTextSize(10);
  glColor3f(1,1,1);
  glLineWidth(2);
  glPointSize(2);
	
  drawText(-WIDTH/2+15,HEIGHT/2 - 10,"WASD     MOVE\nQE       ZOOM\nSPACE   SHOOT\nH        STOP\nESCAPE   QUIT");

  setTextAlign(TEXT_RIGHT);
  drawText(WIDTH/2 - 15, HEIGHT/2 - 10, fps_buf);
}

void drawSpace(cpSpace *space)
{
  cpSpaceEachShape(space, drawShape, NULL);
}

#define SW (4000)
void drawStars()
{
  glColor3f(1,1,1);

  glPushMatrix();
  glTranslatef(((int)player->p.x+SW/2) / SW * SW,((int)player->p.y+SW/2) / SW * SW,0);
	
  glBegin(GL_QUADS);
  for (i=0;i<star_count;i++) {
		float size = stars_size[i];
    glVertex2f(stars_x[i]-size,stars_y[i]-size);
    glVertex2f(stars_x[i]+size,stars_y[i]-size);
    glVertex2f(stars_x[i]+size,stars_y[i]+size);
    glVertex2f(stars_x[i]-size,stars_y[i]+size);
  }
  glEnd();

  glPopMatrix();
}

void drawShape(cpShape *shape, void *unused)
{
  void (*functionPtr)(cpShape*);
  functionPtr = (void (*)(cpShape*))cpShapeGetUserData(shape);
  if(functionPtr != NULL){
    functionPtr(shape);
  }
}

void drawBall(cpShape *shape)
{
  cpCircleShape *circle = (cpCircleShape *)shape;

  drawCircle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), 10,cam_zoom, RGBAColor(0.80f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
}
void drawBox(cpShape *shape)
{
   cpPolyShape *poly = (cpPolyShape *)shape;
   drawPolygon(poly->numVerts, poly->tVerts,RGBAColor(0.80f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
}
void drawground(cpShape *shape)
{
  cpSegmentShape *seg = (cpSegmentShape *)shape;
  drawSegment(seg->ta, seg->tb, seg->r, RGBAColor(0.80f, 0.107f, 0.05f,1.0f));
}

void player_draw(cpShape *shape)
{
  cpCircleShape *circle = (cpCircleShape *)shape;
  drawCircle(circle->tc, cpBodyGetAngle(cpShapeGetBody(shape)), 15,cam_zoom, RGBAColor(0.95f, 0.107f, 0.05f,1.0f),RGBAColor(1.0f, 1.0f, 1.0f,1.0f));
  float s = 0.001;
  float dir = cpBodyGetAngle(cpShapeGetBody(shape));
	
	glLineWidth(2);
	setTextAlign(TEXT_LEFT); // \n is currently only supported by left aligned text
	setTextSize(8);
	setTextAngleRad(dir);
	drawText(circle->tc.x,circle->tc.y, "-THE QUICK BROWN FOX\n+JUMPS OVER\nTHE LAZY DOG\n0123456789");
  /*
	//Font performance test
	for (i = 1; i < 100; i++) {
	setTextAngleRad(dir*i*1.618033988749895f/100.0f);
	setTextSize(s);
	drawText(circle->tc.x,circle->tc.y, "THE QUICK BROWN FOX\nJUMPED OVER THE LAZY DOG\n0123456789");
	s=(10*s+s*1.618033988749895f)/11;
  }
  */
}

void initBall() 
{
  cpVect gravity = cpv(0, -200);
  
  space = cpSpaceNew();
  cpSpaceSetGravity(space, gravity);
  //init stars
  srand(122531);
  for (i=0;i<star_count;i++) {
    stars_x[i] = rand()%(SW*2) - SW;
    stars_y[i] = rand()%(SW*2) - SW;
		stars_size[i] = 2 + 4*(rand() % 1000) / 1000.0f;
  }

  /* static ground */
  cpBody  *staticBody = space->staticBody;
  cpShape *shape;
  shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-2000,-240), cpv(2000,-240), 0.0f));
  cpShapeSetUserData(shape, drawground);
  cpShapeSetFriction(shape, 0.8);
  cpFloat radius = 40;
  cpFloat mass = 1;
  
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  //player
  player = cpSpaceAddBody(space, cpBodyNew(10, cpMomentForCircle(2, 0, 15, cpvzero)));
  cpBodySetPos(player, cpv(30*20,10+1*30));

  cpShape *boxShape = cpSpaceAddShape(space, cpCircleShapeNew(player, 15, cpvzero));
  cpShapeSetFriction(boxShape, 0.7);
  cpShapeSetUserData(boxShape, player_draw);

  for(i = 1; i<20; i++){
    for(j = 1; j<10; j++){
      cpBody *boxBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(1.0f, 30.0f, 30.0f)));

      cpBodySetPos(boxBody, cpv(j*42, i*42));
		  
      cpShape *boxShape = cpSpaceAddShape(space, cpBoxShapeNew(boxBody, radius, radius));
      cpShapeSetFriction(boxShape, 0.7);
      cpShapeSetUserData(boxShape, drawBox);
    }
  }

}

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
  initBall();
  initFont();
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

void game_destroy()
{
  //cpSpaceFreeChildren(space);
  cpSpaceDestroy(space);
  draw_destroy();
  destroyFont();
}
