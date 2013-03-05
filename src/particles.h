#ifndef PARTICLES_H_
#define PARTICLES_H_

#include "chipmunk.h"





typedef struct color {
	float r,g,b;
	float offset;
} color;

typedef struct range {
	float min,max;
} range;

typedef struct particle {

	float x;
	float y;
	float velx;
	float vely;

	float angle;

	float size;

	float time_alive;
	float max_time;

} particle;

typedef struct emitter {

	particle particles[500];

	/** texture id */
	int texture_id;

	/** boolean values */
	int enabled;
	int additive;
	int rotation;
	int infinite; /* = 1 if it spawns particles continuously */
	int emitCount_enabled;/* only spawns emitcount amount of particles and then disables */

	float next_spawn; /* time to next spawn */

	float x,y;

	int particle_count; /* number of particles this emitter have emitted*/
	int max_particles; /* max particles emitted */

	range spawn_interval; /* interval between spawning particles */
	range spawn_count;    /* number of particles to spawn this interval */
	range init_life;      /* lifetime for a particle */
	range init_size;      /* start size of particle */
	range xoffset;        /* spawn particles inside x+-offset */
	range yoffset;
	range init_distance;       /* distancs from x,y to spawn */
	range speed;          /* speed for particle */
	range emitCount;         /* if infinite = 0 it is the time the emitter emits */

	float spread;         /* the angle of spread */
	float angular_offset; /* offset to add to spread angle */


	float growthfactor;   /* particles size = size*growthfactor * 0.001 */
	float gravityfactor;        /* gravity for emitter */
	float windfactor;           /* wind */
	float startalpha;
	float endalpha; /* alpha start and end value */

	color colors[10]; /* a list of colors that the particles get the color from */
	int color_counter;


} emitter;


int read_emitter_from_file (emitter *emi);
void particles_init();
void particles_destroy();
void particles_add_explosion(cpVect v, float time, int speed, int numPar, int color);
void particles_draw();
void particles_update();
void particles_removeall();


#endif /* PARTICLES_H_ */
