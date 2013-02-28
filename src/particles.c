#include "particles.h"

#include "constants.h"
#include "draw.h"
#include "space.h"

#define MAX_PARTICLES 1000
#define MIN_PARTICLES 1
#define MAX_EXPLOSIONS 200
#define MAX_EXPLOSION_TIME 0.4f

const float PARTICLE_SIZE = sizeof(GLfloat[2]);

struct explosion {
	int alive;
	int numParticles;

	GLfloat particle_pos[MAX_PARTICLES*2];
	GLfloat particle_vel[MAX_PARTICLES*2];
	Color color;
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
void particles_removeall()
{
	int i;
	for(i = 0; i<MAX_EXPLOSIONS; i++){
		explosions[i].alive = 0;
	}
	current = 0;
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
		for(i = 0; i < (expl->numParticles)*2-1; i++){
			expl->particle_pos[i] += (expl->particle_vel[i]*dt);
			expl->particle_vel[i]*=0.8f;
			++i;
			expl->particle_pos[i] += (expl->particle_vel[i]*dt);
			expl->particle_vel[i]*=0.8f;
		}
}

static void paricles_explosion_draw(struct explosion *expl)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnable( GL_TEXTURE_2D );

	glEnable(GL_POINT_SPRITE);

	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glPointSize(30);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	//glActiveTexture(GL_TEXTURE0);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glVertexPointer(2, GL_FLOAT, PARTICLE_SIZE, expl->particle_pos);


	Color c = expl->color;
	float fadeStart = 0.3f;
	if(expl->timer > expl->tim_alive -fadeStart){
		c.a = (MAX_EXPLOSION_TIME - expl->timer) / (fadeStart);
		if(c.a < 0){
			c.a = 0;
		}
	}

	glColor4f(c.r, c.g, c.b, c.a);

	int num = expl->numParticles;
	glDrawArrays(GL_POINTS, 0, num);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_TEXTURE_2D);
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
	explosions[current].color = draw_col_grad(col);
	int i;
	for(i = 0; i < num*2 - 1; i++){
		float sp = rand() % speed;
		float angle = RAND_FLOAT * 2 * M_PI;
		explosions[current].particle_pos[i] = v.x;
		explosions[current].particle_vel[i] = cos(angle)*sp;
		++i;
		explosions[current].particle_pos[i] = v.y;
		explosions[current].particle_vel[i] = sin(angle)*sp;
	}
	current++;
}
