#include <stdio.h>
#include <stdlib.h>
#include "shape.h"
#include "cJSON.h"
#include "SDL_log.h"
#include "../io/waffle_utils.h"
#include "../data/hashmap.h"

static void shape_remove(void*);

hashmap *names;

polyshape * polyshape_read(char *filename)
{
	if(names == NULL){
		names = hm_create();
	}
	polyshape * p = hm_get(names, filename);
	if(p != NULL){
		//SDL_Log("FOUND SHAPE IN HASHMAP!");
		return p;
	}

	p = malloc(sizeof(polyshape));
	p->shapes = 0;
	p->polylist = llist_create();
	llist_set_remove_callback(p->polylist,shape_remove);

	char file_path[100];
	sprintf(file_path,"shapes/%s", filename);
	char buff[10000];

	int filesize = waffle_read_file(file_path, buff, 10000);

	if (filesize == 0) {
		return NULL;
	}

	int i;
	cJSON *root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[polyshape] could not parse: %s", filename);
		exit(1);
	}
	cJSON *bodies = cJSON_GetObjectItem(root,"rigidBodies");
	for (i = 0; i < cJSON_GetArraySize(bodies); i++){
		cJSON *body = cJSON_GetArrayItem(bodies, i);
		cJSON *polygons = cJSON_GetObjectItem(body, "polygons");
		int k;
		for (k=0; k < cJSON_GetArraySize(polygons); k++){
			cJSON *polygon = cJSON_GetArrayItem(polygons, k);
			shape_instance *data = calloc(1, sizeof(shape_instance));
			int size = cJSON_GetArraySize(polygon);
			data->num = size;
			data->shape = calloc(size, sizeof(cpVect));
			int j;
			for (j = 0; j <size; j++){
				cJSON *vertex = cJSON_GetArrayItem(polygon, j);
				data->shape[j].x = (cJSON_GetObjectItem(vertex, "x")->valuedouble - 0.5);
				data->shape[j].y = (cJSON_GetObjectItem(vertex, "y")->valuedouble - 0.5);
			}
			for (j = 0; j <size/2; j++) {
				cpVect t = data->shape[j];
				data->shape[j] = data->shape[size-1-j];
				data->shape[size-1-j] = t;
			}
			p->shapes += 1;
			llist_add(p->polylist, data);
		}
	}
	hm_add(names, filename, p);
	return p;
}

void polyshape_add_shapes(cpSpace *space, polyshape *p, cpBody * body, int size, float friction, float elasticity, int group, int type, int layer)
{
	llist_begin_loop(p->polylist);
	while(llist_hasnext(p->polylist)){
		shape_instance *data = (shape_instance*)llist_next(p->polylist);
		cpVect d[data->num];
		int i;
		for(i=0; i<data->num; i++){
			d[i] = data->shape[i];
			d[i] = cpvmult(d[i],size);
		}
		cpShape *sh = cpPolyShapeNew(body, data->num, d, cpv(0,0));
		cpShapeSetFriction(sh, friction);
		cpShapeSetElasticity(sh, elasticity);
		cpShapeSetGroup(sh, group);
		cpShapeSetCollisionType(sh, type);
		cpShapeSetLayers(sh, layer);
		cpSpaceAddShape(space, sh);
	}
	llist_end_loop(p->polylist);
}

static void shape_remove(void *data)
{
	shape_instance *s = (shape_instance*)data;
	free(s->shape);
	free(s);
}

void polyshape_destroy(polyshape *p)
{
	llist_destroy(p->polylist);
	free(p);
}
