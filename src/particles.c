#include "particles.h"

#include "constants.h"
#include "draw.h"
#include "space.h"

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
	float tim_alive;
};

static struct explosion explosions[MAX_EXPLOSIONS];
static int current = 0;
static void paricles_explosion_draw(struct explosion *expl);
static void paricles_explosion_update(struct explosion *expl );

void particles_init()
{
	
}

void particles_destroy()
{
	
}

void particles_draw()
{
	int i;
	for(i = 0; i<MAX_EXPLOSIONS; i++){
		if(explosions[i].alive){
			paricles_explosion_draw(&explosions[i]);
		}
	}
}

void particles_update()
{
	int i;
	for(i = 0; i<MAX_EXPLOSIONS; i++){
		if(explosions[i].alive){
			paricles_explosion_update(&explosions[i]);
		}
	}
}

static void paricles_explosion_update(struct explosion *expl)
{
	expl->timer += dt;

		if(expl->timer > MAX_EXPLOSION_TIME){
			expl->alive = 0;
			expl->timer = 0;
			return;
		}

		int i;
		for(i = 0; i < (expl->numParticles); i++){
			expl->particles[i].pos = cpvadd(expl->particles[i].pos, cpvmult(expl->particles[i].vel,dt));
			if(expl->particles[i].pos.y < 0){
				expl->particles[i].vel.y = -expl->particles[i].vel.y;
				expl->particles[i].pos.y = 10;
			}else if(expl->particles[i].pos.y > level_height){
				expl->particles[i].vel.y = -expl->particles[i].vel.y;
				expl->particles[i].pos.y = level_height-10;
			}
			expl->particles[i].vel = cpvadd(expl->particles[i].vel,cpvmult(cpSpaceGetGravity(space),dt));
			//cpVect tp = expl->particles[i].pos;

			//cpVect tv = expl->particles[i].vel;
		}
}

static void paricles_explosion_draw(struct explosion *expl)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	int i;
	for(i = 0; i < (expl->numParticles); i++){
		cpVect tp = expl->particles[i].pos;
		cpVect tv = expl->particles[i].vel;
		Color c = expl->particles[i].color;
		float fadeStart = 0.3f;
		if(expl->timer > expl->tim_alive -fadeStart){
			c.a = (MAX_EXPLOSION_TIME - expl->timer) / (fadeStart);
			expl->particles[i].vel.x*=0.85f;
			expl->particles[i].vel.y*=0.85f;
			if(c.a < 0){
				c.a = 0;
			}
		}
		glColor4f(c.r, c.g, c.b, c.a);
		draw_line(tp.x, tp.y, tp.x + tv.x/20, tp.y + tv.y/20, (MAX_EXPLOSION_TIME - expl->timer)*120);
	}
	glPopAttrib();
}

void particles_add_explosion(cpVect v , float time, int speed ,int num,int col)
{
	if (current >= MAX_EXPLOSIONS) {
		current = 0;
	}

	num = num > MAX_PARTICLES ? MAX_PARTICLES : (num < 0 ? MIN_PARTICLES : num);

	explosions[current].alive = 1;
	explosions[current].numParticles = num;
	explosions[current].tim_alive = time;
	int i;
	for(i = 0; i < num; i++){
		explosions[current].particles[i].speed =  rand() % speed;
		explosions[current].particles[i].pos.x = v.x;
		explosions[current].particles[i].pos.y = v.y;
		explosions[current].particles[i].vel = cpvmult(cpvforangle(RAND_FLOAT * 2 * M_PI) ,explosions[current].particles[i].speed );

		explosions[current].particles[i].color = draw_col_grad(col);
	
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