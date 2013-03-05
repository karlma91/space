#include "particles.h"

#include "constants.h"
#include "draw.h"
#include "space.h"

/** mini xml reader compatible with anci c */
#include <mxml.h>

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

/**
 * parse functions
 */
static int parse_color_step(mxml_node_t *node, emitter *e);
static int parse_range(mxml_node_t *node, range *r);
static int parse_value(mxml_node_t *node, float *v);


void particles_init()
{
    emitter em;
    em.color_counter = 0;
    read_emitter_from_file (&em);
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

#define TESTNAME(s) strcmp(node->value.element.name, s) == 0
/**
 * reads from a xml file made with pedegree slick2d particle editor
 */
int read_emitter_from_file (emitter *emi)
{

         FILE *fp  = NULL;

         mxml_node_t * tree = NULL;
         mxml_node_t * node  = NULL;

         fp = fopen ("particles/flame.xml", "r");
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


