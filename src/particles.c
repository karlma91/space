#include "particles.h"

#include "constants.h"
#include "draw.h"
#include "space.h"

/** mini xml reader compatible with anci c */
#include <mxml.h>

#define MAX_PARTICLES 10000

/**
 * parse functions
 */
static int read_emitter_from_file (emitter *emi,char *filename);
static int parse_color_step(mxml_node_t *node, emitter *e);
static int parse_range(mxml_node_t *node, range *r);
static int parse_value(mxml_node_t *node, float *v);
static int parse_bool(mxml_node_t *node,char *name, int *v);

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
int available_counter;
static particle *(available_stack[MAX_PARTICLES]);


void particles_init()
{
    emitters[EMITTER_FLAME][0].color_counter = 0;
    read_emitter_from_file (&(emitters[EMITTER_FLAME][0]),"particles/flame_3.xml");
    emitters[EMITTER_FLAME][0].alive = 1;
    emitters[EMITTER_FLAME][0].texture_id = 1;
    emitters[EMITTER_FLAME][0].next_spawn = 0;

    emitters[EMITTER_FLAME][0].head = NULL;


    int i;
    for(i=1; i<10; i++){
    	emitters[EMITTER_FLAME][i] = emitters[EMITTER_FLAME][i-1];
    }
    /* sets all particles available */
    available_counter = 0;
    for(i=0; i<MAX_PARTICLES-1; i++){
    	set_particle_available(&(main_pool[i]));
    }
    //fprintf(stderr,"sizeis correct: %f  %f test %f \n",emitters[EMITTER_FLAME][0].init_life.min,emitters[EMITTER_FLAME][0].init_life.max, range_get_random(emitters[EMITTER_FLAME][0].init_life));
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
	//fprintf(stderr,"update %f %f \n",em->time_allive, em->next_spawn);
	if(em->time_allive >= em->next_spawn){
		emitter_interval(em);
		em->time_allive = 0;
	}
	em->time_allive += mdt;
	emitter_update_particles(em);
}

/**
 * update all particles used by a emitter
 */
static void emitter_update_particles(emitter *em)
{
	particle **prev = &(em->head);
	particle *p = em->head;
	while(p){
		if(p->time_alive >= p->max_time)
		{
			p->alive = 0;
			*prev = p->next;

			p->next = NULL;
			set_particle_available(p);
			p = *prev;
		}else{
			p->vely -= em->gravityfactor * 0.0005f * mdt;
			p->velx += em->windfactor * 0.0005f * mdt;

			particle_update(p);

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
	++available_counter;
	available_stack[available_counter] = p;
}

/**
 * get a particle from the available pool and put it in the in_use list
 * if the pool is empty, then it returns available_pool[0]
 */
static particle * get_particle()
{
	if(available_counter == 0){
		return NULL;
	}else{
		particle *p = available_stack[available_counter];
		available_stack[available_counter] = NULL;
		--available_counter;
		return p;
	}
}

/**
 * update time and position on a single particle
 */
static void particle_update(particle *p)
{

	p->x += p->velx * mdt;
	//fprintf(stderr,"particle: %f -- %f\n",p->x, p->velx);
	p->y += p->vely * mdt;
	p->time_alive += mdt;
}

#include "objects.h"
/**
 * adds a particle to en emitter and sets correct parameters for the particle
 */
static void add_particle(emitter *em)
{
	//em->x = 0;
	//em->y = 400;

	object *pl = objects_first(ID_PLAYER);

	em->x = pl->body->p.x;
	em->y = pl->body->p.y;

	particle *p = get_particle();
	if(p == NULL){
		return;
	}
	p->next = em->head;
	em->head = p;

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


static void draw_particle(particle p, color c){

}

unsigned int particles_active = 0;

static void draw_emitter(emitter *em)
{
	particles_active = 0;
	particle *p = em->head;
	while(p){
		if(p->time_alive < p->max_time){
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
			glPushMatrix();
			glTranslatef(p->x, p->y, 0.0f);
			glRotatef(0,0,0,1);

			glScalef(p->size,p->size,1);
			glBindTexture(GL_TEXTURE_2D, textures[0]);

			glBegin(GL_QUAD_STRIP);
			glTexCoord2d(0, 0); glVertex2d(-0.5, -0.5);
			glTexCoord2d(0, 1); glVertex2d(-0.5, 0.5);
			glTexCoord2d(1, 0); glVertex2d(0.5, -0.5);
			glTexCoord2d(1, 1); glVertex2d(0.5, 0.5);
			glEnd();

			glPopMatrix();
			glPopAttrib();
			glDisable(GL_TEXTURE_2D);
		}

		p = p->next;
	}
}
void particles_draw()
{
	int i,j;
	for(i=0; i<EMITTER_COUNT; i++){
		for (j = 0; j < max_emitters; ++j) {
			if(emitters[i][j].alive){
				//draw_emitter(&(emitters[i][j]));
			}
		}
	}
	draw_emitter(&(emitters[EMITTER_FLAME][0]));
}

void particles_removeall()
{
	//TODO:
}

void particles_update()
{

	int i,j;
	for(i=0; i<EMITTER_COUNT; i++){
		for (j = 0; j < max_emitters; ++j) {
			if(emitters[i][j].alive){
			//	emitter_update(&(emitters[i][j]));
			}
		}
	}
	emitter_update(&(emitters[EMITTER_FLAME][0]));

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
        		 if(TESTNAME("system")){
        			 parse_bool(node,"additive",&(emi->additive));
        		 }else if(TESTNAME("emitter")){
        			 parse_bool(node,"useAdditive",&(emi->additive));
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
static int parse_bool(mxml_node_t *node, char *name, int *v)
{
    int ok = 0;
    int k;
    for (k = 0; k < node->value.element.num_attrs; k++){
    	if(strcmp(node->value.element.attrs[k].name, name) == 0){
    		if(strcmp(node->value.element.attrs[k].value,"true")){
    			*v = 0;
    		}else{
    			*v = 1;
    		}
    		ok++;
    	}
    }
    if(ok == 1){
    	return 0;
    }else{
    	fprintf(stderr,"Error parsing boolean in node %s \n", node->value.element.name);
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
