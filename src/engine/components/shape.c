#include <stdio.h>
#include <stdlib.h>
#include "shape.h"
#include "cJSON.h"
#include "SDL_log.h"
#include "../io/waffle_utils.h"
#include "../data/hashmap.h"

static void rigid_body_remove(void*);
static vertex_array* read_vertex_array(cJSON * array);
static void vertex_array_free(void *data);

hashmap *names;

polyshape shape_read(char *filename)
{
	if(names == NULL){
		names = hm_create();
	}
	polyshape polys = hm_get(names, filename);
	if(polys != NULL){
		//SDL_Log("FOUND SHAPE IN HASHMAP!");
		return polys;
	}

	polys = calloc(sizeof(*polys), 1);
	polys->pshape = llist_create();
	strcpy(polys->name, filename);
	llist_set_remove_callback(polys->pshape, rigid_body_remove);

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

		polygon_ins *pi = calloc(1, sizeof(polygon_ins));
		pi->shape = llist_create();
		llist_set_remove_callback(pi->shape, vertex_array_free);
		pi->outlines = llist_create();
		llist_set_remove_callback(pi->outlines, vertex_array_free);

		cJSON *polygons = cJSON_GetObjectItem(body, "polygons");
		int k;
		for (k=0; k < cJSON_GetArraySize(polygons); k++){
			cJSON *polygon = cJSON_GetArrayItem(polygons, k);
			vertex_array *data = read_vertex_array(polygon);
			llist_add(pi->shape, data);
		}
		cJSON *shapes = cJSON_GetObjectItem(body, "shapes");
		for (k=0; k < cJSON_GetArraySize(shapes); k++) {
			cJSON *shape = cJSON_GetArrayItem(shapes, k);
			cJSON *vertices = cJSON_GetObjectItem(shape, "vertices");
			vertex_array *data = read_vertex_array(vertices);
			llist_add(pi->outlines, data);
		}
		llist_add(polys->pshape, pi);
	}
	cJSON_Delete(root);
	hm_add(names, filename, polys);
	return polys;
}

static vertex_array* read_vertex_array(cJSON * array)
{
	vertex_array *va = calloc(1, sizeof(vertex_array));
	int size = cJSON_GetArraySize(array);
	va->num = size;
	va->vertices = calloc(size, sizeof(cpVect));
	int i;
	for (i = 0; i <size; i++){
		cJSON *vertex = cJSON_GetArrayItem(array, i);
		va->vertices[i].x = (cJSON_GetObjectItem(vertex, "x")->valuedouble - 0.5);
		va->vertices[i].y = (cJSON_GetObjectItem(vertex, "y")->valuedouble - 0.5);
	}
	for (i = 0; i <size/2; i++) {
		cpVect t = va->vertices[i];
		va->vertices[i] = va->vertices[size-1-i];
		va->vertices[size-1-i] = t;
	}
	return va;
}

void shape_add_shapes(cpSpace *space, polyshape ps, cpBody * body, int size, cpVect offset, float friction, float elasticity, cpGroup group, cpCollisionType type, cpLayers layer, unsigned int shapes)
{
	LList p = ps->pshape;
	if (!p) {
		SDL_Log("ERROR: polyshape pointer to NULL in add_shapes!");
		//TODO add a default shape instead
		return;
	}
	int addall = shapes == 0 ? 1 : 0;
	llist_begin_loop(p);
	while (llist_hasnext(p)) {
		polygon_ins *pi = (polygon_ins*) llist_next(p);
		if (addall || (shapes & 0x1)) {
			llist_begin_loop(pi->shape);
			while (llist_hasnext(pi->shape)) {
				vertex_array *data = (vertex_array*) llist_next(pi->shape);
				cpVect d[data->num];
				int i;
				for (i = 0; i < data->num; i++) {
					d[i] = data->vertices[i];
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
			llist_end_loop(pi->shape);
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
	polygon_ins * rb = (polygon_ins*)data;
	llist_destroy(rb->shape);
	llist_destroy(rb->outlines);
	free(rb);
}

static void vertex_array_free(void *data)
{
	vertex_array *va = (vertex_array*)data;
	free(va->vertices);
}

void shape_destroy(polyshape p)
{
	//hm_destroy(names);
	llist_destroy(p);
	free(p);
}
