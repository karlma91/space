#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "particles.h"
#include "draw.h"
#include "constants.h"

struct particle {
	cpVect pos;
	cpVect vel;
	Color color;
	cpFloat speed;
};

struct explosion {
	int alive;
	int numParticles;
	struct particle particles[MAX_PARTICLES];
	float timer;
};

static int i;
static struct explosion explosions[MAX_EXPLOSIONS];
static int current = 0;
static void paricles_explosion_draw(struct explosion *expl,float dt);


void paricles_init()
{
	
}

void paricles_destroy()
{
	
}

void paricles_draw(float dt)
{
	for(i = 0; i<MAX_EXPLOSIONS; i++){
		if(explosions[i].alive){
			paricles_explosion_draw(&explosions[i], dt);	
		}
	}
}

static void paricles_explosion_draw(struct explosion *expl , float dt)
{
	int i = 0;
	expl->timer += dt;

	if(expl->timer > MAX_EXPLOSION_TIME){
		expl->alive = 0;
		expl->timer = 0;
		return;
	}

	for(i = 0; i<expl->numParticles; i++){
		expl->particles[i].pos = cpvadd(expl->particles[i].pos,expl->particles[i].vel);
		if(expl->particles[i].pos.y < 0){
			expl->particles[i].vel.y = -expl->particles[i].vel.y;
			expl->particles[i].pos.y = 0;
		}
		cpVect tp = expl->particles[i].pos;
		cpVect tv = expl->particles[i].vel;

		//glColor_from_color(expl->particles[i].color);
		Color c = expl->particles[i].color;
		glColor4f(c.r,c.g,c.b,  (MAX_EXPLOSION_TIME - expl->timer)/MAX_EXPLOSION_TIME);
		draw_line(tp.x, tp.y, tp.x + tv.x*3, tp.y + tv.y*3, 200 - expl->particles[i].speed*2 );
	}
	
}

void paricles_add_explosion(cpVect v, int num)
{
	
	current = current > MAX_EXPLOSIONS ? 0 : current;
	num = num < 0 ? 5 : num;
	num = num > MAX_PARTICLES ? MAX_PARTICLES : num;

	explosions[current].alive = 1;
	explosions[current].numParticles = num;

	for(i = 0; i < num; i++){
		explosions[current].particles[i].speed =  rand() % 100;
		explosions[current].particles[i].pos.x = v.x;
		explosions[current].particles[i].pos.y = v.y;
		explosions[current].particles[i].vel = cpvmult(cpvforangle(RAND_FLOAT * 2 * M_PI) ,explosions[current].particles[i].speed );

		explosions[current].particles[i].color = draw_rainbow_col(rand());
	
		/*	if(ran == 0){
			explosions[current].particles[i].color = RGBAColor(1,
								   RAND_FLOAT,
								   RAND_FLOAT,
								   1.0f);
		}else if(ran == 1){
			explosions[current].particles[i].color = RGBAColor(RAND_FLOAT,
								   1,
								   RAND_FLOAT,
								   1.0f);	
		}else{
			explosions[current].particles[i].color = RGBAColor(RAND_FLOAT,
								   RAND_FLOAT,
								   1,
								   1.0f);	
								   }*/
		
		
	}
	current++;
}

