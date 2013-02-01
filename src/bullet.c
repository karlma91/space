#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "math.h"
#include "bullet.h"

static void init(object *bullet);
static void update(object *bullet);
static void render(object *bullet);
static void destroy(object *bullet);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);

