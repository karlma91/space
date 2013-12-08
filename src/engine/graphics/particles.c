
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
static int parse_range(mxml_node_t *node, range *r);
static int parse_point(mxml_node_t *node, p_point *point);
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
static struct pool * main_emitter_pool;
static struct pool *main_particle_pool;

unsigned int particles_active = 0;

static int emitter_type_count = 0;
static hashmap *hm_emitter_names;

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
	hm_emitter_names = hm_create();
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
	if (!s) return;
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
	if (!s) return;
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
	if (!s) return;
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

emitter *particles_get_emitter_at(particle_system *s, int layer, EMITTER_ID type, cpVect p)
{
	if (!s) return NULL;
	emitter *e = particles_get_emitter(s, layer, type);
	e->p = p;
	return e;
}

emitter *particles_get_emitter(particle_system *partl_sys, int layer, EMITTER_ID type)
{
	if (!partl_sys) return NULL;
	if (type->ID != type) {
		SDL_Log("ERROR: Invalid emitter template!");
		return NULL;
	}
	emitter *e = get_emitter();
	llist_add(partl_sys->emitters, e);
	LList ll_particles = e->particles;
	*e = *type;
	e->particles = ll_particles;
	e->layer = layer;
	if (e->emit_count_enabled) {
		e->emit_count_set = range_get_random(e->emit_count);
	}
	if (e->length_enabled) {
		e->length_set = range_get_random(e->length);
	}
	e->alive = 1;
	e->self_draw = 0;
	e->partl_sys = partl_sys;
	return e;
}

/* do not call on emitters that has length or count enabled outside particles.c */
void particles_release_emitter(emitter* e)
{
	if(e != NULL){
		e->waiting_to_die = 1;
	}
}

/** set as default emitter->particles remove_callback */
static void clear_rm_particle(particle *p)
{
	p->alive = 0;
	set_particle_available(p);
}
/** set as default system->emitters remove_callback */
static void clear_emitter_particles(emitter *e)
{
	llist_clear(e->particles);
}
static void clear_emitter_available(emitter *e)
{
	llist_clear(e->particles);
	set_emitter_available(e);
}

/**
 * destroys a system, puts all emitters available and frees system
 */
void particlesystem_free(particle_system *s) {
	/* clears emitter list */
	if (!s)
		return;

	llist_set_remove_callback(s->emitters, (ll_rm_callback) clear_emitter_available);
	llist_destroy(s->emitters);
	free(s);
}

/**
 * resets all emitters and particles
 */
void particles_destroy()
{
	llist_begin_loop(main_emitter_pool->available);
	while(llist_hasnext(main_emitter_pool->available)) {
		emitter *e = llist_next(main_emitter_pool->available);
		if(e->particles != NULL) {
			llist_destroy(e->particles);
			e->particles = NULL;
		}
	}
	llist_end_loop(main_emitter_pool->available);
	// FIXME: fix crash on destroy
	//pool_destroy(main_emitter_pool);
	//pool_destroy(main_particle_pool);

	hashiterator *it = hm_get_iterator(hm_emitter_names);
	while (hm_iterator_hasnext(it)) {
		free(hm_iterator_next(it));
	}
	hm_iterator_destroy(it);
	hm_destroy(hm_emitter_names);
}

/**
 * resets all the active particles
 */
void particles_clear(particle_system *s)
{
	//FIXME
	//llist_set_remove_callback(s->emitters, (ll_rm_callback) clear_emitter_particles);
	//llist_clear(s->emitters);
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
	if (!em->waiting_to_die) {
		if (em->time_allive >= em->next_spawn && !em->disable) {
			emitter_interval(em);
			em->time_allive = 0;
		}
	}else{
		if (llist_size(em->particles) == 0) {
			em->alive = 0;
		}
	}

	if (em->length_enabled && em->total_time_allive >= em->length_set) {
		em->waiting_to_die = 1;
	}

	if (em->emit_count_enabled && em->particle_count >= em->emit_count_set) {
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
		if (p->time_alive >= p->max_time) {
			p->alive = 0;
			llist_remove(em->particles, p); // Sets particle available with remove callback
		} else {
		    cpVect g = em->partl_sys->gravity_dir_func(p->p);

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
 * get an emitter from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static emitter *get_emitter(void)
{
	emitter *e = pool_instance(main_emitter_pool);
	if(e == NULL) {
        SDL_Log("Particke get_emiiter ERROR");
	} else {
        if(e->particles == NULL) {
            e->particles = llist_create();
            llist_set_remove_callback(e->particles, (ll_rm_callback) clear_rm_particle );
        }
    }
	return e;
}

/**
 * get a particle from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static particle * get_particle(void)
{
	particle *p = pool_instance(main_particle_pool);
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

	llist_begin_loop(em->particles);
	while(llist_hasnext(em->particles)){
		particle *p = llist_next(em->particles);

		++particles_active;
		float offset = p->time_alive / p->max_time;
		offset = offset>1 ? 1 : offset;

		Color col = {0,0,0,0};
		int i;

		p_point p1 = em->alpha[0];
		for(i = 0; i<em->alpha_count-1; i++) {
			p_point p0 = p1; p1 = em->alpha[i+1];
			if (offset >= p0.x && offset <= p1.x) {
				float alpha_offset = (offset - p0.x) / (p1.x - p0.x);
				float alpha_inv =  1 - alpha_offset;
				col.a = p0.y * alpha_inv + p1.y * alpha_offset;
			}
		}

		p_color b = em->colors[0];
		for(i = 0; i<em->color_count-1; i++) {
			p_color a = b; b = em->colors[i+1];
			if(offset >= a.offset && offset <= b.offset){
				float step = b.offset - a.offset;
				float coloffset = (offset - a.offset) / step;
				float colinv = 1 - coloffset;

				col.r = 0xFF * (a.r * colinv + b.r * coloffset);
				col.g = 0xFF * (a.g * colinv + b.g * coloffset);
				col.b = 0xFF * (a.b * colinv + b.b * coloffset);
			}
		}
		draw_color(col);
		em->draw_particle(em, p);
	}
	llist_end_loop(em->particles);

	draw_pop_color();
}

static void default_particle_draw(emitter *em, particle *p)
{
	float angle = 0;
	sprite_render(em->layer, &p->spr, p->p, p->angle + angle);
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
EMITTER_ID read_emitter_from_file(const char *filename)
{
	EMITTER_ID id = particles_bind_emitter(filename);
	if (id) return id->ID; //TODO be able to re-load emitter templates

	id = calloc(1, sizeof(emitter));
	emitter *em = (emitter *) id;
	em->ID = id;
	em->alpha_count = 0;
	em->color_count = 0;
	em->alive = 1;
	em->disable = 0;
	em->next_spawn = 0;
	em->particle_count = 0;
	em->waiting_to_die = 0;
	em->draw_particle = default_particle_draw;

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
		free(em);
		return -1;
	}
	if(tree == NULL){
		SDL_Log("particles.c: file is empty \n");
		free(em);
		return -1;
	}

	int alpha_last = 0; //TODO go through/check child or parent node instead of using state variable

	for (node = mxmlFindElement(tree, tree,NULL,NULL, NULL,MXML_DESCEND);
			node != NULL;
			node=mxmlWalkNext (node, NULL, MXML_DESCEND)
	){
		if (node->type  == MXML_ELEMENT) {
			if (alpha_last) {
				int index = em->alpha_count;
				if (index < PARTICLE_ALPHA_MAX && (parse_point(node, em->alpha + index) == 0)) {
					++em->alpha_count;
					//fprintf(stderr, "DEBUG: added point = {%f, %f}\n", (em->alpha+index)->x, (em->alpha+index)->y);
				} else {
					alpha_last = 0;
					//fprintf(stderr, "DEBUG: alpha_last = 0\n");
				}
			}

			if(TESTNAME("system")){
				parse_bool(node,"additive",&(em->additive));
			}else if(TESTNAME("emitter")){
				char *(spint[1]);
				parse_string(node,"spriteName",spint);
				//SDL_Log( "HELLO TEXTURE: %s\n", *spint);
				if(*spint != NULL){
					em->sprite_id = sprite_link(*spint);
				}
				//parse_bool(node,"useAdditive",&(em->additive));
				parse_bool(node,"useOriented",&(em->rotation));
			}else if(TESTNAME("spawnInterval")){
				parse_range(node,&(em->spawn_interval));
			}else if(TESTNAME("spawnCount")){
				parse_range(node,&(em->spawn_count));
			}else if(TESTNAME("initialLife")){
				parse_range(node,&(em->init_life));
			}else if(TESTNAME("initialSize")){
				parse_range(node,&(em->init_size));
			}else if(TESTNAME("initialRotation")){
				parse_range(node,&(em->init_rotation));
			}else if(TESTNAME("rotationSpeed")){
				parse_range(node,&(em->speed_rotation));
			}else if(TESTNAME("xOffset")){
				parse_range(node,&(em->xoffset));
			}else if(TESTNAME("yOffset")){
				parse_range(node,&(em->yoffset));
			}else if(TESTNAME("initialDistance")){
				parse_range(node,&(em->init_distance));
			}else if(TESTNAME("speed")){
				parse_range(node,&(em->speed));
			}else if(TESTNAME("emitCount")){
				parse_range(node,&(em->emit_count));
				parse_bool(node,"enabled",&(em->emit_count_enabled));
			}else if(TESTNAME("length")){
				parse_range(node,&(em->length));
				parse_bool(node,"enabled",&(em->length_enabled));
			}else if(TESTNAME("spread")){
				parse_float(node,"value", &(em->spread));
				em->spread /= WE_180_PI; //convert to radians
			}else if(TESTNAME("angularOffset")){
				parse_float(node,"value",&(em->angular_offset));
				em->angular_offset /= WE_180_PI; //convert to radians
			}else if(TESTNAME("growthFactor")){
				parse_float(node,"value",&(em->growthfactor));
			}else if(TESTNAME("gravityFactor")){
				parse_float(node,"value",&(em->gravityfactor));
			}else if(TESTNAME("windFactor")){
				parse_float(node,"value",&(em->windfactor));
			}else if(TESTNAME("alpha")){
				alpha_last = 1;
				//fprintf(stderr, "DEBUG: alpha_last = 1\n");
			}else if(TESTNAME("color")){
			}else if(TESTNAME("step")){
				parse_color_step(node,em);
			}else{
				// SDL_Log("unused element %s \n", node->value.element.name);
			}

		}else {
			// SDL_Log("Type Default Node is %s \n", node->value.element.name);
		}
	}
	mxmlDelete(tree);
	emitter_type_count++;
	hm_add(hm_emitter_names, filename, em);
	return em->ID;
}

EMITTER_ID particles_bind_emitter(const char *name)
{
	return (EMITTER_ID) hm_get(hm_emitter_names, name);
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
 * Parses the atributes of a point
 * return 0 on ok, else -1
 */
static int parse_point(mxml_node_t *node, p_point *point)
{
	if (TESTNAME("point")) {
		return parse_float(node,"x", &(point->x)) || parse_float(node,"y", &(point->y));
	} else {
		return -1;
	}
}

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
static int parse_color_step(mxml_node_t *node, emitter *e)
{
	int index = e->color_count;
	if (index < PARTICLE_COLOR_MAX) {
		parse_float(node,"r", &(e->colors[index].r));
		parse_float(node,"g", &(e->colors[index].g));
		parse_float(node,"b", &(e->colors[index].b));
		parse_float(node,"offset", &(e->colors[index].offset));
		(e->color_count)++;
		return 0;
	} else {
		return -1;
	}
}
