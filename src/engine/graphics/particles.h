#ifndef PARTICLES_H_
#define PARTICLES_H_

#include "chipmunk.h"
#include "sprite.h"


typedef struct particle_color {
	float r,g,b;
	float offset;
} p_color;

typedef struct range {
	float min,max;
} range;

typedef struct particle particle;
typedef struct emitter emitter;
struct particle {

	int alive;

	cpVect p;
	cpVect v;
	float angle;
	float rot_speed;
	float size;

	sprite spr;

	float time_alive;
	float max_time;

	particle *next;
};

struct emitter {

	int type;

	SPRITE_ID sprite_id;

	/** particle draw functions **/
	void (*draw_particle)(emitter *em, particle *p);

	/** particle list **/
	int list_length;
	particle *head;

	/** boolean values */
	int alive;
	int waiting_to_die;
	int disable;

	int additive;
	int rotation;
	int infinite; /* = 1 if it spawns particles continuously */
	int emit_count_enabled;/* only spawns emitcount amount of particles and then disables */
	int length_enabled;/* only spawns emitcount amount of particles and then disables */

	float time_allive; /* counter for spawn intervals */
	float total_time_allive;
	float next_spawn; /* time to next spawn */

	cpVect p;

	int particle_count; /* number of particles this emitter have emitted*/

	float emit_count_set; /* current emit  */
	float length_set;

	range spawn_interval; /* interval between spawning particles */
	range spawn_count;    /* number of particles to spawn this interval */
	range init_life;      /* lifetime for a particle */
	range init_size;      /* start size of particle */
	range init_rotation;
	range speed_rotation;
	range xoffset;        /* spawn particles inside x+-offset */
	range yoffset;
	range init_distance;       /* distancs from x,y to spawn */
	range speed;          /* speed for particle */
	range emit_count;         /* if infinite = 0 it is the time the emitter emits */
	range length;         /* if infinite = 0 it is the time the emitter emits */

	float spread;         /* the angle of spread */
	float angular_offset; /* offset to add to spread angle */


	float growthfactor;   /* particles size = size*growthfactor * 0.001 */
	float gravityfactor;        /* gravity for emitter */
	float windfactor;           /* wind */
	float startalpha;
	float endalpha; /* alpha start and end value */

	p_color colors[10]; /* a list of colors that the particles get the color from */
	int color_counter;

	/** data to use in a custom draw function */
	void *data;

	emitter *next;
};


int read_emitter_from_file (char *filename);
void particles_init(void);
void particles_destroy(void);
void particles_draw(void);
void particles_update(void);
void particles_release_emitter(emitter* e);

emitter *particles_add_score_popup(cpVect p,int score);
emitter *particles_add_sparks(cpVect p, float angle, float force);

void particles_clear(void);
emitter *particles_get_emitter(int type);
emitter *particles_get_emitter_at(int type,cpVect p);

void particles_reload_particles(void);

extern unsigned int particles_active;
extern int available_particle_counter; //TMP

#endif /* PARTICLES_H_ */
