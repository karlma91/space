
#include "particles.h"
#include "draw.h"
#include "mxml.h"
#include "../io/xmlh.h"
#include "../io/waffle_utils.h"
#include "../engine.h"
#include "we_utils.h"
#include "we_data.h"

#define PARTICLE_READ_BUFFER_SIZE 4096

/**
 * parse functions
 */
int read_emitter_from_file (char *filename);
static int parse_range(mxml_node_t *node, range *r);
static int parse_color_step(mxml_node_t *node, emitter *e);

/**
 * emitter functions
 */
static float range_get_random(range r);
static void emitter_interval(emitter *em);
static void emitter_update(emitter *em);

static void update_all_particles(emitter *em);
static void draw_all_particles(emitter *em);

/**
 * Particle draw functions
 */
static void default_particle_draw(emitter *em, particle *p);

static void particle_update_pos(particle *p);

static emitter * get_emitter(void);
static void set_emitter_available(emitter *e);

static cpVect default_gravity_func(cpVect pos) {
    return cpv(0,-1);
}

/**
 * stack operators
 */
static void add_particle(emitter *em);
static particle * get_particle();
static void set_particle_available(particle *p);

/*
 * variables
 */
static int current_emitter = -1;

static struct pool * main_emitter_pool;

//TODO: make emitter_templates dynamic size
static emitter emitter_templates[40];

unsigned int particles_active = 0;

static struct pool *main_particle_pool;

/**
 * GLOABL FUNCTIONS
 */

/**
 * Resets everything and load files to templates.
 */
void particles_init(void)
{
	main_emitter_pool = pool_create(sizeof(emitter));
	main_particle_pool = pool_create(sizeof(particle));
}

particle_system *particlesystem_new()
{
	particle_system * s = (particle_system *)calloc(1, sizeof *s);
	s->emitters = llist_create();
	s->gravity_dir_func = default_gravity_func;
	return s;
}

void particles_update(particle_system *s)
{
	llist_begin_loop(s->emitters);
	while(llist_hasnext(s->emitters)){
		emitter *e = llist_next(s->emitters);
		if(e->alive == 0) {
			llist_remove(s->emitters, e);
			set_emitter_available(e);
		} else {
			emitter_update(e);
		}
	}
	llist_end_loop(s->emitters);
}

void particle_set_gravity_func(particle_system *s, cpVect (*gravity_dir_func)(cpVect p))
{
    if(gravity_dir_func != NULL){
        s->gravity_dir_func = gravity_dir_func;
    }
}

void particles_self_draw(emitter *e, int enable)
{
	if(e != NULL) {
		e->self_draw = enable;
	}
}

void particles_draw_emitter(emitter *e)
{
	if(e) {
		draw_all_particles(e);
	}
}

void particles_draw(particle_system *s)
{
	particles_active = 0;
	llist_begin_loop(s->emitters);
	while(llist_hasnext(s->emitters)) {
		emitter *e = llist_next(s->emitters);
		if(e->self_draw == 0) {
			draw_all_particles(e);
		}
	}
	llist_end_loop(s->emitters);
}

emitter *particles_get_emitter_at(particle_system *s, int type, cpVect p)
{
	emitter *e = particles_get_emitter(s, type);
	e->p = p;
	return e;
}


emitter *particles_get_emitter(particle_system *s, int type)
{
	emitter *e = get_emitter();
	llist_add(s->emitters, e);
	void * l = e->particles;
	*e = (emitter_templates[type]);
	e->particles = l;
	if(e->emit_count_enabled){
		e->emit_count_set = range_get_random(e->emit_count);
	}
	if(e->length_enabled){
		e->length_set = range_get_random(e->length);
	}
	e->alive = 1;
	e->self_draw = 0;
	e->ps = s;
	return e;
}

/* do not call on emitters that has length or count enabled outside particles.c */
void particles_release_emitter(emitter* e)
{
	if(e != NULL){
		e->waiting_to_die = 1;
	}
}

/**
 * destroys a system, puts all emitters available and frees system
 */
void particlesystem_free(particle_system *s) {
	/* clears emitter list */
	if (!s)
		return;

	particles_clear(s);
	llist_begin_loop(s->emitters);
	while(llist_hasnext(s->emitters)) {
		emitter *e = llist_next(s->emitters);
		set_emitter_available(e);
	}
	llist_end_loop(s->emitters);
	llist_destroy(s->emitters);
	free(s);
}

/**
 * resets all emitters and particles
 */
void particles_destroy()
{
	/*llist_begin_loop(main_emitter_pool->available);
	while(llist_hasnext(main_emitter_pool->available)) {
		emitter *e = llist_next(main_emitter_pool->available);
		if(e->particles){
			llist_destroy(e->particles);
			e->particles = NULL;
		}
	}
	llist_end_loop(main_emitter_pool->in_use);
	llist_begin_loop(main_emitter_pool->in_use);
	while(llist_hasnext(main_emitter_pool->in_use)) {
		emitter *e = llist_next(main_emitter_pool->in_use);
		if(e->particles){
			llist_destroy(e->particles);
			e->particles = NULL;
		}
	}
	llist_end_loop(main_emitter_pool->in_use);
	*/
	//pool_destroy(main_emitter_pool);
	//pool_destroy(main_particle_pool);
}

/**
 * resets all the active particles
 */
void particles_clear(particle_system *s)
{
	llist_begin_loop(s->emitters);
	while (llist_hasnext(s->emitters)) {
		emitter *e = llist_next(s->emitters);
		if (e != NULL) {
			llist_begin_loop(e->particles);
			while(llist_hasnext(e->particles)){
				particle *p = llist_next(e->particles);
				llist_remove(e->particles, p);
				p->alive = 0;
				set_particle_available(p);
			}
			llist_end_loop(e->particles);
		}
	}
	llist_end_loop(s->emitters);
}

/**
 * ***********************
 *
 *
 * PRIVAT FUNCTIONS
 *
 *
 * ***********************
 */


/**
 * adds a emitter back to the stack
 */
static void set_emitter_available(emitter *e)
{
	pool_release(main_emitter_pool, e);
}

static void emitter_interval(emitter *em)
{
	float spaw_count = range_get_random(em->spawn_count);
	em->next_spawn = range_get_random(em->spawn_interval);
	em->time_allive = 0;
	int i;
	for(i=0; i<spaw_count; i++){
		em->particle_count++;
		add_particle(em);
	}
}

static void emitter_update(emitter *em)
{
	if(!em->waiting_to_die){
		if(em->time_allive >= em->next_spawn && !em->disable){
			emitter_interval(em);
			em->time_allive = 0;
		}
	}else{
		if(llist_size(em->particles) == 0){
			em->alive = 0;
		}
	}

	if(em->length_enabled && em->total_time_allive >= em->length_set){
		em->waiting_to_die = 1;
	}

	if(em->emit_count_enabled && em->particle_count >= em->emit_count_set){
		em->waiting_to_die = 1;
	}

	update_all_particles(em);
	em->time_allive += mdt;
	em->total_time_allive += mdt;
}

/**
 * update all particles used by a emitter
 */
static void update_all_particles(emitter *em)
{
	llist_begin_loop(em->particles);
	while(llist_hasnext(em->particles)){
		particle *p = llist_next(em->particles);
		if(p->time_alive >= p->max_time) {
			p->alive = 0;
			llist_remove(em->particles, p);
			set_particle_available(p);
		} else {
		    cpVect g = em->ps->gravity_dir_func(p->p);

		    p->v = cpvadd(p->v, cpvmult(g, em->gravityfactor * 0.0001f * mdt));

		   // cpVect wind =  cpvmult(cpvperp(g), em->windfactor * 0.0001f * mdt);
		   // p->v = cpvadd(p->v, wind);

		    if(em->rotation){
		        p->angle += p->rot_speed*dt;
		    }
			particle_update_pos(p);
		}
	}
	llist_end_loop(em->particles);
}

/**
 * adds a particle back to the stack
 */
static void set_particle_available(particle *p)
{
	pool_release(main_particle_pool, p);
}

/**
 * get a particle from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static emitter * get_emitter(void)
{
	emitter *e = pool_instance(main_emitter_pool);
	if(e == NULL) {
			SDL_Log("Particke get_emiiter ERROR");
	}
	if(e->particles == NULL) {
		e->particles = llist_create();
	}
	return e;
}

/**
 * get a particle from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static particle * get_particle(void)
{
	particle *p = pool_instance(main_emitter_pool);
	if(p == NULL) {
		SDL_Log("Particke get_particle ERROR");
	}
	return p;
}

/**
 * update time and position on a single particle
 */
static void particle_update_pos(particle *p)
{
	p->p = cpvadd(p->p, cpvmult(p->v, mdt));
	p->time_alive += mdt;
}

/**
 * adds a particle to en emitter and sets correct parameters for the particle
 */
static void add_particle(emitter *em)
{
	particle *p = get_particle();
	llist_add(em->particles, p);

	p->alive = 1;

	/* speed */
	float angle = em->spread * (we_randf - 0.5) + em->angular_offset + WE_PI_2;
	float speed = range_get_random(em->speed) * 0.001f;
	cpVect t = cpvforangle(angle);
	p->v = cpvmult(t,speed);

	/* position */
	p->p = cpvadd(em->p,cpvmult(t,range_get_random(em->init_distance)));

	/* initial size */
	p->size = range_get_random(em->init_size);
	p->angle = range_get_random(em->init_rotation);
	p->rot_speed = range_get_random(em->speed_rotation);
	/* set time to live */
	p->max_time = range_get_random(em->init_life);
	p->time_alive = 0;
	sprite_create(&p->spr, em->sprite_id,p->size, p->size, 0);
}


/**
 * returns a random number inside a range
 */
static float range_get_random(range r)
{
	return r.min + we_randf * (r.max-r.min);
}

static void draw_all_particles(emitter *em)
{
	draw_push_color();
	draw_push_blend();
	if(em->additive){
		draw_blend(GL_SRC_ALPHA, GL_ONE);
	}

	llist_begin_loop(em->particles);
	while(llist_hasnext(em->particles)){
		particle *p = llist_next(em->particles);

		++particles_active;
		float offset = p->time_alive / p->max_time;
		offset = offset>1 ? 1 : offset;
		float inv = 1-offset;

		p_color a,b,c;
		float coloffset;
		float colinv;
		int i;
		float alpha = (em->startalpha/255.0f)*inv + (em->endalpha/255.0f)*offset;
		for(i = 0; i<em->color_counter-1; i++) {
			a = em->colors[i];
			b = em->colors[i+1];
			if(offset >= a.offset && offset <= b.offset){

				float step = b.offset - a.offset;

				coloffset = (offset - a.offset) / step;
				colinv = coloffset;
				coloffset = 1 - coloffset;

				c.r = (a.r * coloffset) + (b.r * colinv);
				c.g = (a.g * coloffset) + (b.g * colinv);
				c.b = (a.b * coloffset) + (b.b * colinv);
			}
		}
		draw_color4f(c.r,c.g,c.b,alpha);
		em->draw_particle(em, p);
	}
	llist_end_loop(em->particles);

	if(em->draw_particle == default_particle_draw) {
		draw_flush_color();
	}
	draw_pop_blend();
	draw_pop_color();

}

static void default_particle_draw(emitter *em, particle *p)
{
	float angle = 0;
	sprite_render(&p->spr, p->p, p->angle + angle);
}


/***************************************
 *
 *
 *
 *
 * PRIVATE PARSER STUFF
 *
 *
 *
 *
 **************************************/

/**
 * reads from a xml file made with pedegree slick2d particle editor
 */
int read_emitter_from_file (char *filename)
{
	current_emitter += 1;
	emitter *emi = &(emitter_templates[current_emitter]);
	emi->type = current_emitter;
	emi->color_counter = 0;
	emi->alive = 1;
	emi->disable = 0;
	emi->next_spawn = 0;
	emi->particle_count = 0;
	emi->waiting_to_die = 0;
	emi->draw_particle = default_particle_draw;

	mxml_node_t * tree = NULL;
	mxml_node_t * node  = NULL;

	char particle_path[200];
	sprintf(particle_path, "particles/%s", filename);

	char buffer[PARTICLE_READ_BUFFER_SIZE];
	int filesize = waffle_read_file(particle_path, buffer, PARTICLE_READ_BUFFER_SIZE);

	if (filesize) {
		tree = mxmlLoadString(NULL, buffer, MXML_OPAQUE_CALLBACK);
	}else {
		SDL_Log("Could Not Open the File Provided");
		return 1;
	}
	if(tree == NULL){
		SDL_Log("particles.c: file is empty \n");
		return 1;

	}
	for (node = mxmlFindElement(tree, tree,NULL,NULL, NULL,MXML_DESCEND);
			node != NULL;
			node=mxmlWalkNext (node, NULL, MXML_DESCEND)
	){
		if (node->type  == MXML_ELEMENT) {
			if(TESTNAME("system")){
				parse_bool(node,"additive",&(emi->additive));
			}else if(TESTNAME("emitter")){
				char *(spint[1]);
				parse_string(node,"spriteName",spint);
				//SDL_Log( "HELLO TEXTURE: %s\n", *spint);
				if(*spint != NULL){
					emi->sprite_id = sprite_link(*spint);
				}
				parse_bool(node,"useAdditive",&(emi->additive));
				parse_bool(node,"useOriented",&(emi->rotation));
			}else if(TESTNAME("spawnInterval")){
				parse_range(node,&(emi->spawn_interval));
			}else if(TESTNAME("spawnCount")){
				parse_range(node,&(emi->spawn_count));
			}else if(TESTNAME("initialLife")){
				parse_range(node,&(emi->init_life));
			}else if(TESTNAME("initialSize")){
				parse_range(node,&(emi->init_size));
			}else if(TESTNAME("initialRotation")){
				parse_range(node,&(emi->init_rotation));
			}else if(TESTNAME("rotationSpeed")){
				parse_range(node,&(emi->speed_rotation));
			}else if(TESTNAME("xOffset")){
				parse_range(node,&(emi->xoffset));
			}else if(TESTNAME("yOffset")){
				parse_range(node,&(emi->yoffset));
			}else if(TESTNAME("initialDistance")){
				parse_range(node,&(emi->init_distance));
			}else if(TESTNAME("speed")){
				parse_range(node,&(emi->speed));
			}else if(TESTNAME("emitCount")){
				parse_range(node,&(emi->emit_count));
				parse_bool(node,"enabled",&(emi->emit_count_enabled));
			}else if(TESTNAME("length")){
				parse_range(node,&(emi->length));
				parse_bool(node,"enabled",&(emi->length_enabled));
			}else if(TESTNAME("spread")){
				parse_float(node,"value", &(emi->spread));
				emi->spread /= WE_180_PI; //convert to radians
			}else if(TESTNAME("angularOffset")){
				parse_float(node,"value",&(emi->angular_offset));
				emi->angular_offset /= WE_180_PI; //convert to radians
			}else if(TESTNAME("growthFactor")){
				parse_float(node,"value",&(emi->growthfactor));
			}else if(TESTNAME("gravityFactor")){
				parse_float(node,"value",&(emi->gravityfactor));
			}else if(TESTNAME("windFactor")){
				parse_float(node,"value",&(emi->windfactor));
			}else if(TESTNAME("startAlpha")){
				parse_float(node,"value",&(emi->startalpha));
			}else if(TESTNAME("endAlpha")){
				parse_float(node,"value",&(emi->endalpha));
			}else if(TESTNAME("color")){

			}else if(TESTNAME("step")){
				parse_color_step(node,emi);
			}else{
				// SDL_Log("unused element %s \n", node->value.element.name);
			}

		}else {
			// SDL_Log("Type Default Node is %s \n", node->value.element.name);
		}
	}

	mxmlDelete(tree);
	return current_emitter;
}



/**
 * Parses the atributes of a node to a range r
 * return 0 on ok, else -1
 */
static int parse_range(mxml_node_t *node, range *r)
{
	parse_float(node,"min", &(r->min));
	parse_float(node,"max", &(r->max));
	return 0;
}


/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
static int parse_color_step(mxml_node_t *node, emitter *e)
{
	int index = e->color_counter;
	(e->color_counter)++;
	parse_float(node,"r", &(e->colors[index].r));
	parse_float(node,"g", &(e->colors[index].g));
	parse_float(node,"b", &(e->colors[index].b));
	parse_float(node,"offset", &(e->colors[index].offset));
	return 0;
}
