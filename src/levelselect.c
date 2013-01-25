#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"


static void init();
static void update();
static void render();
static void destroy();

/* extern */
state state_levelselect = {
		init,
		update,
		render,
		destroy,
		NULL
};
static void init()
{

}

static void update()
{

}

static void render()
{

}

static void destroy()
{

}
