#include "particles.h"

#include "constants.h"
#include "draw.h"
#include "space.h"

/** mini xml reader compatible with anci c */
#include <mxml.h>
/** helper functions for mxml **/
#include "xmlh.h"

#define MAX_PARTICLES 10000
#define MAX_EMITTERS 10

/**
 * parse functions
 */
static int read_emitter_from_file (emitter *emi,char *filename);
static int parse_range(mxml_node_t *node, range *r);
static int parse_color_step(mxml_node_t *node, emitter *e);
/**
 * emitter functions
 */
static float range_get_random(range r);
static void emitter_interval(emitter *em);
static void update_all_particles();
static void emitter_update(emitter *em);
static void particle_update_pos(particle *p);

static emitter * get_emitter();
static void set_emitter_available(emitter *e);


/**
 * stack operators
 */
static void add_particle(emitter *em);
static particle * get_particle();
static void set_particle_available(particle *p);

static emitter main_emitter_pool[MAX_EMITTERS];
static emitter emitter_templates[EMITTER_COUNT];
static emitter *emitters_in_use_list;
static emitter *(available_emitter_stack[MAX_EMITTERS]);
static int available_emitter_counter;

unsigned int particles_active = 0;

static particle main_partice_pool[MAX_PARTICLES];
int available_particle_counter;
static particle *particles_in_use;
static particle *(available_particle_stack[MAX_PARTICLES]);

/**
 * Resets everything and load files to templates.
 */
void particles_init()
{
    int i;
    read_emitter_from_file(&(emitter_templates[EMITTER_FLAME]),"particles/flame_3.xml");
    read_emitter_from_file(&(emitter_templates[EMITTER_EXPLOTION]),"particles/explosion_ground.xml");
    read_emitter_from_file(&(emitter_templates[EMITTER_SPARKS]),"particles/sparks.xml");
    read_emitter_from_file(&(emitter_templates[EMITTER_SMOKE]),"particles/smoke.xml");

    /* sets in use list empty */
    emitters_in_use_list = NULL;
    particles_in_use = NULL;

    available_particle_counter = -1;
    /* sets all particles available */
    for(i=0; i<MAX_PARTICLES; i++){
    	main_partice_pool[i].next = NULL;
    	set_particle_available(&(main_partice_pool[i]));
    }
    available_emitter_counter = -1;
    /* sets all emitters available */
    for(i=0; i<MAX_EMITTERS; i++){
    	available_emitter_stack[i] = NULL;
    	main_emitter_pool[i].next = NULL;
    	set_emitter_available(&(main_emitter_pool[i]));
    }
}

/**
 * resets all emitters and particles
 */
void particles_destroy()
{
	particles_clear();

	/* clears emitter list */
	emitter *e = emitters_in_use_list;
	while(e){
		set_emitter_available(e);
		e = e->next;
	}
	emitters_in_use_list = NULL;
}

/**
 * resets all the active particles
 */
void particles_clear()
{
    particle *p = particles_in_use;
    while(p){
    	set_particle_available(p);
    	p = p->next;
    }
    particles_in_use = NULL;
}


emitter *particles_get_emitter(int type)
{
	emitter *e = get_emitter();
	if(e != NULL){
		emitter *next = e->next;
		*e = (emitter_templates[type]);
		if(e->emit_count_enabled){
			e->emit_count_set = range_get_random(e->emit_count);
		}
		if(e->length_enabled){
			e->length_set = range_get_random(e->length);
		}
		e->next = next;
		return e;
	}else{
		return NULL;
	}
}

/* do not call on emitters that has length or count enabled outside particles.c */
void particles_release_emitter(emitter* e)
{
	if(e != NULL){
		e->alive = 0;
	}
}

/**
 * adds a emitter back to the stack
 */
static void set_emitter_available(emitter *e)
{
	e->alive = 0;
	e->next = NULL;
	++available_emitter_counter;
	available_emitter_stack[available_emitter_counter] = e;
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
	//fprintf(stderr,"update %f %f \n",em->time_allive, em->next_spawn);
	if(em->time_allive >= em->next_spawn){
		emitter_interval(em);
		em->time_allive = 0;
	}

	if(em->length_enabled && em->total_time_allive > em->length_set){
		em->alive = 0;
	}
	if(em->emit_count_enabled && em->particle_count > em->emit_count_set && em->alive){
		em->alive = 0;
	}

	em->time_allive += mdt;
	em->total_time_allive += mdt;
}


void update_all_emitters(){
	emitter **prev = &(emitters_in_use_list);
	emitter *e = emitters_in_use_list;
	while(e){
		if(e->alive == 0)
		{
			*prev = e->next;
			set_emitter_available(e);
			e = *prev;
		}else{
			emitter_update(e);
			prev = &(e->next);
			e = e->next;
		}
	}
}

/**
 * update all particles used by a emitter
 */
static void update_all_particles()
{
	particle **prev = &(particles_in_use);
	particle *p = particles_in_use;
	while(p){
		emitter *em = p->e;
		if(p->time_alive >= p->max_time)
		{
			p->alive = 0;
			*prev = p->next;
			set_particle_available(p);
			p = *prev;
		}else{
			p->vely -= em->gravityfactor * 0.0001f * mdt;
			p->velx += em->windfactor * 0.0001f * mdt;

			particle_update_pos(p);

			prev = &(p->next);
			p = p->next;
		}
	}
}

/**
 * adds a particle back to the stack
 */
static void set_particle_available(particle *p)
{
	p->next = NULL;
	++available_particle_counter;
	available_particle_stack[available_particle_counter] = p;
}

/**
 * get a particle from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static emitter * get_emitter()
{
	if(available_emitter_counter == -1){
		return NULL;
	}else{
		emitter *e = available_emitter_stack[available_emitter_counter];
		available_emitter_stack[available_emitter_counter] = NULL;
		--available_emitter_counter;

		e->alive = 1;

		/* puts emitter in inuse list */
		e->next = emitters_in_use_list;
		emitters_in_use_list = e;
		return e;
	}
}

/**
 * get a particle from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static particle * get_particle()
{
	if(available_particle_counter == 0){
		return NULL;
	}else{
		particle *p = available_particle_stack[available_particle_counter];
		available_particle_stack[available_particle_counter] = NULL;
		--available_particle_counter;
		return p;
	}
}

/**
 * update time and position on a single particle
 */
static void particle_update_pos(particle *p)
{
	p->x += p->velx * mdt;
	p->y += p->vely * mdt;
	p->time_alive += mdt;
}

/**
 * adds a particle to en emitter and sets correct parameters for the particle
 */
static void add_particle(emitter *em)
{
	particle *p = get_particle();
	if(p == NULL){
		return;
	}
	p->next = particles_in_use;
	particles_in_use = p;

	p->e = em;
	p->alive = 1;

	/* speed */
	float angle = ((RAND_FLOAT - 0.5)*((em->spread)) + (em->angular_offset+90))*(M_PI/180);
	float speed = range_get_random(em->speed) * 0.001f;
	p->velx = cos(angle)*speed;
	p->vely = sin(angle)*speed;

	/* position */
	p->x = em->x;
	p->y = em->y;

	/* initial size */
	p->size = range_get_random(em->init_size);
	/* set time to live */
	p->max_time = range_get_random(em->init_life);
	p->time_alive = 0;
}


/**
 * returns a random number inside a range
 */
static float range_get_random(range r)
{
	return r.min + ((float)rand()/(float)RAND_MAX)*(r.max-r.min);
}

static void draw_all_particles()
{
	particle *p = particles_in_use;
	while(p){
		emitter *em = p->e;
		++particles_active;
		float offset = p->time_alive / p->max_time;
		float inv = 1-offset;

		color a,b,c;
		float coloffset;
		float colinv;
		int i;
		float alpha = (em->startalpha/255.0f)*inv + (em->endalpha/255.0f)*offset;
		for(i = 0; i<em->color_counter-1; i++){
			a = em->colors[i];
			b = em->colors[i+1];
			if(offset >= a.offset && offset <= b.offset){

				float step = b.offset - a.offset;

				coloffset = offset - a.offset;
				coloffset = coloffset / step;
				coloffset = 1 - coloffset;
				colinv = 1 - coloffset;

				c.r = (a.r * coloffset) + (b.r * colinv);
				c.g = (a.g * coloffset) + (b.g * colinv);
				c.b = (a.b * coloffset) + (b.b * colinv);
			}
		}

		glColor4f(c.r,c.g,c.b,alpha);
		glEnable(GL_TEXTURE_2D);
		glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
		if(em->additive){
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}

		if(em->rotation){
			//float angle = (atan2(p->vely,p->velx) + M_PI)*(180/M_PI);
			//glRotatef(angle,0,0,1);
			draw_line(p->x,p->y,p->x + p->velx*p->size,p->y+ p->vely*p->size, p->size);
		}else{
			glPushMatrix();
			glTranslatef(p->x, p->y, 0.0f);
			glScalef(p->size,p->size,1);

			texture_bind(em->texture_id);

			glBegin(GL_QUAD_STRIP);
			glTexCoord2d(0, 0); glVertex2d(-0.5, -0.5);
			glTexCoord2d(0, 1); glVertex2d(-0.5, 0.5);
			glTexCoord2d(1, 0); glVertex2d(0.5, -0.5);
			glTexCoord2d(1, 1); glVertex2d(0.5, 0.5);
			glEnd();

			glPopMatrix();
		}
			glPopAttrib();
			glDisable(GL_TEXTURE_2D);

		p = p->next;
	}
}
void particles_draw()
{
	particles_active = 0;
	draw_all_particles();
}

void particles_update()
{
	update_all_particles();
	update_all_emitters();
}

void particles_add_explosion(cpVect v, float time, int speed, int numPar, int color){
	emitter *e = particles_get_emitter(EMITTER_SPARKS);
	if(e){
		e->x = v.x;
		e->y = v.y;
	}
}


/****
 *
 *
 *
 *
 * PARSER STUFF
 *
 *
 *
 *
 */


/**
 * reads from a xml file made with pedegree slick2d particle editor
 */
static int read_emitter_from_file (emitter *emi,char *filename)
{

	emi->color_counter = 0;
	emi->alive = 1;
	emi->next_spawn = 0;
	emi->particle_count = 0;

         FILE *fp  = NULL;

         mxml_node_t * tree = NULL;
         mxml_node_t * node  = NULL;

         fp = fopen (filename, "r");
         if (fp ){
             tree = mxmlLoadFile (NULL , fp , MXML_OPAQUE_CALLBACK);
         }else {
             fprintf(stderr,"Could Not Open the File Provided");
             return 1;
         }
         if(tree == NULL){
        	 fprintf(stderr,"particles.c file is empty \n");
        	 return 1;

         }
         fprintf(stderr,"particles.c parsing %s \n",filename);
         for (node = mxmlFindElement(tree, tree,NULL,NULL, NULL,MXML_DESCEND);
        		 node != NULL;
        		 node=mxmlWalkNext (node, NULL, MXML_DESCEND)
         ){
        	 if (node->type  == MXML_ELEMENT) {
        		 if(TESTNAME("system")){
        			 parse_bool(node,"additive",&(emi->additive));
        		 }else if(TESTNAME("emitter")){
        			 char *(spint[1]);
        			 parse_string(node,"imageName",spint);
        			 fprintf(stderr, "HELLO TEXTURE: %s\n", *spint);
        			 if(*spint != NULL){
        				 emi->texture_id = texture_load(*spint);
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
        		 }else if(TESTNAME("angularOffset")){
        		     parse_float(node,"value",&(emi->angular_offset));
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
        		    // fprintf (stderr,"unused element %s \n", node->value.element.name);
        		 }

        	 }else {
        		// fprintf(stderr,"Type Default Node is %s \n", node->value.element.name);
        	 }
         }

         mxmlDelete(tree);
         fclose(fp);
         return 0;
}



/**
 * Parses the atributes of a node to a range r
 * return 0 on ok, else -1
 */
static int parse_range(mxml_node_t *node, range *r)
{
	parse_float(node,"min",&(r->min));
	parse_float(node,"max",&(r->max));
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
    parse_float(node,"r",&(e->colors[index].r));
    parse_float(node,"g",&(e->colors[index].g));
    parse_float(node,"b",&(e->colors[index].b));
    parse_float(node,"offset",&(e->colors[index].offset));

    return 0;
}
