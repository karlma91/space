#include "particles.h"

#include "constants.h"
#include "draw.h"
#include "space.h"

/** mini xml reader compatible with anci c */
#include <mxml.h>

#define MAX_PARTICLES 1000

/**
 * parse functions
 */
static int read_emitter_from_file (emitter *emi,char *filename);
static int parse_color_step(mxml_node_t *node, emitter *e);
static int parse_range(mxml_node_t *node, range *r);
static int parse_value(mxml_node_t *node, float *v);

/**
 * emitter functions
 */
static float range_get_random(range r);
static void emitter_interval(emitter *em);
static void emitter_update_particles(emitter *em);
static void emitter_update(emitter *em);
static void particle_update(particle *p);
/**
 * stack operators
 */
static void add_particle(emitter *em);
static particle * get_particle();
static void set_particle_available(particle *p);

static emitter emitters[EMITTER_COUNT][10];
static int max_emitters = 10;
static int flame_count;
static int explotion_count;
static particle main_pool[MAX_PARTICLES];
static int available_counter;
static particle *(available_stack[MAX_PARTICLES]);


void particles_init()
{
    emitters[EMITTER_FLAME][0].color_counter = 0;
    read_emitter_from_file (&(emitters[EMITTER_FLAME][0]),"particles/flame.xml");
    emitters[EMITTER_FLAME][0].alive = 1;
    emitters[EMITTER_FLAME][0].texture_id = 1;
    int i;
    for(i=1; i<10; i++){
    	emitters[EMITTER_FLAME][i] = emitters[EMITTER_FLAME][i-1];
    }
    /* sets all particles available */
    for(i=0; i<MAX_PARTICLES; i++){
    	available_stack[i] = &(main_pool[i]);
    }
    available_counter = MAX_PARTICLES;
}

void particles_destroy()
{
	
}

emitter * particles_get_emitter(int type)
{
	switch (type) {
	case EMITTER_FLAME:
		if(flame_count>max_emitters){
			flame_count = 0;
		}
		return &(emitters[EMITTER_FLAME][flame_count++]);
		break;
	case EMITTER_EXPLOTION:
		if(explotion_count>max_emitters){
			explotion_count = 0;
		}
		return &(emitters[EMITTER_EXPLOTION][explotion_count++]);
		break;
	default:
		break;
	}
	return NULL;
}

static void emitter_interval(emitter *em)
{
	float spaw_count = range_get_random(em->spawn_count);
	em->next_spawn = range_get_random(em->spawn_interval);
	em->time_allive = 0;
	em->alive = 1;

	int i;
	for(i=0; i<spaw_count; i++){
		add_particle(em);
	}
}

static void emitter_update(emitter *em)
{
	if(em->time_allive >= em->next_spawn){
		emitter_interval(em);
	}
	em->time_allive+=dt;
	emitter_update_particles(em);
}

/**
 * update all particles used by a emitter
 */
static void emitter_update_particles(emitter *em)
{
	particle **prev = em->head;
	particle *p = *(em->head);
	while(p){
		if(p->time_alive >= p->max_time)
		{
			p->alive = 0;
			(*prev) = p->next;
			p->next = NULL;
			p = (*prev);
			set_particle_available(p);
		}else{
			particle_update(p);
		}
	}
}
/**
 * adds a particle back to the stack
 */
static void set_particle_available(particle *p)
{
	available_stack[++available_counter] = p;
}
/**
 * update time and position on a single particle
 */
static void particle_update(particle *p)
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
	p->next = *(em->head);
	*(em->head) = p;

	p->alive = 1;

	/* speed */
	float angle = ((RAND_FLOAT - 0.5)*((em->spread)) + (em->angular_offset+90))*(M_PI/180);
	float speed = range_get_random(em->speed);
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
 * get a particle from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static particle * get_particle()
{
	particle *p = available_stack[available_counter--];
	available_counter = available_counter < 0 ? 0 : available_counter;
	return p;
}

/**
 * returns a random number inside a range
 */
static float range_get_random(range r)
{
	return r.min + ((float)rand()/RAND_MAX)*(r.max-r.min);
}


static void draw_particle(particle p, color c){

}

static void draw_emitter(emitter *em)
{
	particle *p = *(em->head);
	while(p){

		float offset = em->time_allive / em->next_spawn;
		float inv = 1-offset;



	}
}
void particles_draw()
{
	/*
	int i,j;
	for(i=0; i<EMITTER_COUNT; i++){
		for (j = 0; j < max_emitters; ++j) {
			if(emitters[j][i].alive){
				emitter_update(&(emitters[j][i]));
			}
		}
	}*/
}

void particles_removeall()
{
	//TODO:
}

void particles_update()
{
	/*
	int i,j;
	for(i=0; i<EMITTER_COUNT; i++){
		for (j = 0; j < max_emitters; ++j) {
			if(emitters[j][i].alive){
				emitter_update(&(emitters[j][i]));
			}
		}
	}
	*/
}

static void paricles_explosion_update(struct explosion *expl)
{

}

void particles_add_explosion(cpVect v, float time, int speed, int numPar, int color){

}


/****
 *
 *
 * PARSER STUFF
 *
 *
 */
#define TESTNAME(s) strcmp(node->value.element.name, s) == 0

/**
 * reads from a xml file made with pedegree slick2d particle editor
 */
static int read_emitter_from_file (emitter *emi,char *filename)
{

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
        		 if(TESTNAME("emitter")){

        		 }else if(TESTNAME("spawnInterval")){
        		     parse_range(node,&(emi->spawn_interval));
        		 }else if(TESTNAME("spawnCount")){
        		     parse_range(node,&(emi->spawn_count));
        		 }else if(TESTNAME("initialLife")){
        		     parse_range(node,&(emi->init_life));
        		 }else if(TESTNAME("initialSize")){
        		     parse_range(node,&(emi->init_life));
        		 }else if(TESTNAME("xOffset")){
        		     parse_range(node,&(emi->xoffset));
        		 }else if(TESTNAME("yOffset")){
        		     parse_range(node,&(emi->yoffset));
        		 }else if(TESTNAME("initialDistance")){
        		     parse_range(node,&(emi->init_distance));
        		 }else if(TESTNAME("speed")){
        		     parse_range(node,&(emi->speed));
        		 }else if(TESTNAME("emitCount")){
        		     parse_range(node,&(emi->emitCount));
        		 }else if(TESTNAME("spread")){
        		     parse_value(node,&(emi->spread));
        		 }else if(TESTNAME("angularOffset")){
        		     parse_value(node,&(emi->angular_offset));
        		 }else if(TESTNAME("growthFactor")){
        		     parse_value(node,&(emi->growthfactor));
        		 }else if(TESTNAME("gravityFactor")){
        		     parse_value(node,&(emi->gravityfactor));
        		 }else if(TESTNAME("windFactor")){
        		     parse_value(node,&(emi->windfactor));
        		 }else if(TESTNAME("startAlpha")){
        		     parse_value(node,&(emi->startalpha));
        		 }else if(TESTNAME("endAlpha")){
        		     parse_value(node,&(emi->endalpha));
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
    int ok = 0;
    int k;
    for (k = 0; k < node->value.element.num_attrs; k++){
	if(strcmp(node->value.element.attrs[k].name, "min") == 0){
	    r->min = strtod(node->value.element.attrs[k].value,NULL);
	    ok++;
	}else if(strcmp(node->value.element.attrs[k].name, "max") == 0){
	    r->max = strtod(node->value.element.attrs[k].value,NULL);
	    ok++;
	}
    }
    if(ok == 2){
	return 0;
    }else{
	 fprintf(stderr,"Error parsing range in node %s  ok: %d \n", node->value.element.name,ok);
	 return -1;
    }
}

/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
static int parse_value(mxml_node_t *node, float *v)
{
    int ok = 0;
    int k;
    for (k = 0; k < node->value.element.num_attrs; k++){
	if(strcmp(node->value.element.attrs[k].name, "value") == 0){
	    *v = strtod(node->value.element.attrs[k].value,NULL);
	    ok++;
	}
    }
    if(ok == 1){
	return 0;
    }else{
	 fprintf(stderr,"Error parsing value in node %s \n", node->value.element.name);
	 return -1;
    }
}
/**
 * Parses the atributes of a node to a value v
 * return 0 on ok, else -1
 */
static int parse_color_step(mxml_node_t *node, emitter *e)
{
    int index = e->color_counter;
    int ok = 0;
    int k;
    for (k = 0; k < node->value.element.num_attrs; k++){
	if(strcmp(node->value.element.attrs[k].name, "r") == 0){
	    e->colors[index].r = strtod(node->value.element.attrs[k].value,NULL);
	    ok++;
	}else if(strcmp(node->value.element.attrs[k].name, "g") == 0){
	    e->colors[index].g = strtod(node->value.element.attrs[k].value,NULL);
	    ok++;
	}else if(strcmp(node->value.element.attrs[k].name, "b") == 0){
	    e->colors[index].b = strtod(node->value.element.attrs[k].value,NULL);
	    ok++;
	}else if(strcmp(node->value.element.attrs[k].name, "offset") == 0){
	    e->colors[index].offset = strtod(node->value.element.attrs[k].value,NULL);
	    ok++;
	}
    }
    e->color_counter++;

    if(ok == 4){
	return 0;
    }else{
	 fprintf(stderr,"Error parsing color step %s \n", node->value.element.name);
	 return -1;
    }
}
