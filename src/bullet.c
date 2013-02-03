/* header */
#include "bullet.h"

/* Chipmunk physics library */
#include "chipmunk.h"

/* Drawing */
#include "draw.h"
#include "particles.h"

/* Game components */
#include "objects.h"

static void init(object *bullet);
static void update(object *bullet);
static void render(object *bullet);
static void destroy(object *bullet);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);
