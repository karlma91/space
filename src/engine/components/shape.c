#include <stdio.h>
#include <stdlib.h>
#include "shape.h"
#include "cJSON.h"
#include "SDL_log.h"
#include "../io/waffle_utils.h"
#include "../data/hashmap.h"

static void rigid_body_remove(void*);

hashmap *names;

polyshape shape_read(char *filename)
{
	if(names == NULL){
		names = hm_create();
	}
	polyshape p = hm_get(names, filename);
	if(p != NULL){
		//SDL_Log("FOUND SHAPE IN HASHMAP!");
		return p;
	}

	p = llist_create();
	llist_set_remove_callback(p, rigid_body_remove);

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

		LList rb = llist_create();

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
			llist_add(rb, data);
		}
		llist_add(p,rb);
	}
	hm_add(names, filename, p);
	return p;
}

void shape_add_shapes(cpSpace *space, polyshape p, cpBody * body, int size, cpVect offset, float friction, float elasticity, cpGroup group, cpCollisionType type, cpLayers layer, unsigned int shapes)
{
	if (!p) {
		SDL_Log("ERROR: polyshape pointer to NULL in add_shapes!");
		//TODO add a default shape instead
		return;
	}
	int addall = shapes == 0 ? 1 : 0;
	llist_begin_loop(p);
	while (llist_hasnext(p)) {
		LList rb = (LList) llist_next(p);
		if (addall || (shapes & 0x1)) {
			llist_begin_loop(rb);
			while (llist_hasnext(rb)) {
				shape_instance *data = (shape_instance*) llist_next(rb);
				cpVect d[data->num];
				int i;
				for (i = 0; i < data->num; i++) {
					d[i] = data->shape[i];
					d[i] = cpvmult(d[i], size);
				}
				cpShape *sh = cpPolyShapeNew(body, data->num, d, offset);
				cpShapeSetFriction(sh, friction);
				cpShapeSetElasticity(sh, elasticity);
				cpShapeSetGroup(sh, group);
				cpShapeSetCollisionType(sh, type);
				cpShapeSetLayers(sh, layer);
				cpSpaceAddShape(space, sh);
			}
			llist_end_loop(rb);
		}
		shapes = shapes >> 1;
	}
	llist_end_loop(p);
	if (shapes != 0) {
		SDL_Log("ERROR: polyshape trying to  add non existing shape");
	}

}

static void rigid_body_remove(void *data)
{
	LList rb = (LList)data;
	llist_begin_loop(rb);
	while(llist_hasnext(rb)){
		shape_instance *data = (shape_instance*)llist_next(rb);
		free(data->shape);
		free(data);
	}
	llist_end_loop(rb);
	llist_destroy(rb);
	free(rb);
}

void shape_destroy(polyshape p)
{
	hm_destroy(names);
	llist_destroy(p);
	free(p);
}
