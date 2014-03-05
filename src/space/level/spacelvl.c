#include "we.h"
#include "spacelvl.h"
#include "../game.h"

#define FILE_SIZE_BUFFER 128000

//TODO move into utils?
static void remove_static(void *data)
{
	cpShape *shape = data;
	cpSpaceRemoveStaticShape(current_space, shape);
	cpShapeFree(shape);
}

spacelvl *spacelvl_parse(int folder, const char * filename)
{
	SDL_Log("PARSING LEVEL : %s", filename);
	char file_path[100];
	char buff[FILE_SIZE_BUFFER]; // TODO: stor nok? (10kb ikke stort nok!)
	int filesize;

	if(folder == WAFFLE_DOCUMENTS||
			folder == WAFFLE_LIBRARY){
		FILE *f = waffle_fopen(WAFFLE_DOCUMENTS, filename, "r");
		filesize = waffle_read(f, buff, FILE_SIZE_BUFFER);
	}else if(folder == WAFFLE_ZIP ) {
		sprintf(file_path,"level/%s.json", filename);
		filesize = waffle_read_file_zip(file_path, buff, FILE_SIZE_BUFFER);
	}


	if (filesize == 0) {
		SDL_Log("Could not load level %s filesize = 0", filename);
		return NULL;
	}

	spacelvl *lvl = calloc(1, sizeof(spacelvl));
	lvl->ll_recipes = llist_create();
	llist_set_remove_callback(lvl->ll_recipes, objrecipe_free);

	cJSON *root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Level] could not parse level: %s", filename);
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return NULL;
	}

	// GET level name
	strcpy(lvl->name, cJSON_GetObjectItem(root,"name")->valuestring);

	load_tilemap(root, &lvl->tm);

	/* Calculate level size based on grid data */
	lvl->inner_radius = lvl->tm.grid->pol.rad[lvl->tm.grid->pol.inner_i];
	lvl->outer_radius = lvl->tm.grid->pol.rad[lvl->tm.grid->pol.outer_i-1];;
	lvl->height = lvl->outer_radius - lvl->inner_radius;


	cJSON * object_array = cJSON_GetObjectItem(root,"objects");

	objrecipe_load_objects(lvl->ll_recipes,object_array);

    lvl->ll_tileshapes = llist_create();
    llist_set_remove_callback(lvl->ll_tileshapes, remove_static);
    lvl->ceiling = NULL;
	lvl->loaded2space = NULL;

	return lvl;
}


we_bool spacelvl_write(spacelvl *lvl)
{
	char filename[200];
	sprintf(filename, "levels/karlmka/%s.json",lvl->name);
	FILE *file = waffle_fopen(WAFFLE_DOCUMENTS, filename,"w");
	if (file == NULL) {
		SDL_Log( "Could not open %s\n",filename);
		return WE_FALSE;
	}

	cJSON *root;//,*fmt;
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name", cJSON_CreateString(lvl->name));
	cJSON_AddItemToObject(root, "tilemap", cJSON_CreateString("level02_01.tmx"));
	cJSON_AddNumberToObject(root, "timelimit", 100);

	cJSON_AddNumberToObject(root, "t_layers", lvl->tm.layers);
	cJSON_AddNumberToObject(root, "t_cols", lvl->tm.grid->pol.cols);
	cJSON_AddNumberToObject(root, "t_inner_i", lvl->tm.grid->pol.inner_i);
	cJSON_AddNumberToObject(root, "t_outer_i", lvl->tm.grid->pol.outer_i);
	cJSON_AddNumberToObject(root, "t_min_rad", lvl->tm.grid->pol.min_rad);

	cJSON * tilemap = cJSON_CreateArray();
	int ii = lvl->tm.grid->pol.inner_i;
	int i,j,k;
	for (i=0; i < lvl->tm.layers; i++) {
		cJSON * row = cJSON_CreateArray();
		for (j= 0; j < lvl->tm.grid->pol.rows; j++) {
			int temp[lvl->tm.grid->pol.cols];
			for (k = 0; k < lvl->tm.grid->pol.cols; k++) {
				temp[k] = lvl->tm.data[i][j+ii][k];
			}
			cJSON * col = cJSON_CreateIntArray(temp, lvl->tm.grid->pol.cols);
			cJSON_AddItemToArray(row, col);
		}
		cJSON_AddItemToArray(tilemap, row);
	}

	cJSON * param_array = cJSON_CreateArray();

	cJSON * object_array = cJSON_CreateArray();
	llist_begin_loop(lvl->ll_recipes);
	while(llist_hasnext(lvl->ll_recipes)) {
		object_recipe * data = llist_next(lvl->ll_recipes);
		cJSON *object = cJSON_CreateObject();
		cJSON_AddItemToObject(object, "type", cJSON_CreateString(data->obj_type->NAME));
		cJSON_AddItemToObject(object, "name", cJSON_CreateString(data->param_name));
		cJSON *pos = cJSON_CreateObject();
		cJSON_AddNumberToObject(pos,"x",data->pos.x);
		cJSON_AddNumberToObject(pos,"y",data->pos.y);
		cJSON_AddNumberToObject(pos,"r",data->rotation);
		cJSON_AddItemToObject(object,"pos", pos);
		cJSON_AddItemToArray(object_array, object);
	}

	cJSON_AddItemToObject(root,"tilemaptest", tilemap);
	cJSON_AddItemToObject(root,"params", param_array);
	cJSON_AddItemToObject(root,"objects", object_array);

	char * rendered = cJSON_Print(root);
	//SDL_Log("%s", rendered);
	fprintf(file,"%s",rendered);
	fclose(file);
	// TODO: Write to file
	cJSON_Delete(root);

	return WE_TRUE;
}

spacelvl *spacelvl_copy(spacelvl *lvl)
{
	int x, y;
	spacelvl *lvl_dst = calloc(1, sizeof *lvl_dst);
	*lvl_dst = *lvl;
	lvl_dst->ll_tileshapes = llist_create();
    llist_set_remove_callback(lvl_dst->ll_tileshapes, (ll_rm_callback) remove_static);
    lvl_dst->ceiling = NULL;
    lvl_dst->loaded2space = NULL;
	for (y = lvl_dst->tm.grid->pol.inner_i; y < lvl_dst->tm.grid->pol.outer_i; y++)
		for (x = 0; x < lvl_dst->tm.grid->pol.cols; x++)
			lvl_dst->tm.metadata[y][x].block = NULL;
	return lvl_dst;
}

we_bool spacelvl_freecopy(spacelvl **lvl)
{
	if (*lvl) {
		llist_destroy((*lvl)->ll_tileshapes);
		free(*lvl);
		*lvl = NULL;
		return WE_TRUE;
	} else {
		return WE_FALSE;
	}
}

we_bool spacelvl_load2state(spacelvl *lvl)
{
	if (lvl->loaded2space) {
		fprintf(stderr, "ERROR: spacelvl already loaded to a state, pls unload from correct state before loading!\n");
		return WE_FALSE;
	}

	cpResetShapeIdCounter();
	llist_begin_loop(lvl->ll_recipes);
	while(llist_hasnext(lvl->ll_recipes)) {
		object_recipe * data = llist_next(lvl->ll_recipes);
		SDL_Log("INSTANTIATING TYPE: %s at: %f ", data->obj_type->NAME, data->pos.x);
		instance_create(data->obj_type, data->param, data->pos, cpvzero);
	}
	llist_end_loop(lvl->ll_recipes);

	/* static ground */
	cpBody *staticBody = current_space->staticBody;

	int x,y;
	/* add static tiles */
	fprintf(stderr, "GRID: %d x %d\n", lvl->tm.grid->pol.rows, lvl->tm.grid->pol.cols);
	//TODO move into function inside tilemap?
	for (y = lvl->tm.grid->pol.inner_i; y < lvl->tm.grid->pol.outer_i; y++) {
		for (x = 0; x < lvl->tm.grid->pol.cols; x++) {
			byte tile = tilemap_gettype(&lvl->tm, TLAY_SOLID, x, y);
			if (tile) { //TODO support different types of shapes and use a helper method both here and in editor
				int len = 4;
				cpVect verts[4];
				grid_getquad8cpv(lvl->tm.grid, verts, x, y);
				switch (tile) {
				case TILE_TYPE_DIAG_SEL:
					verts[0] = verts[1];
					verts[1] = verts[2];
					verts[2] = verts[3];
					verts[0] = cpvmult(cpvadd(verts[0],verts[1]), 0.5);
					len = 3;
					break;
				case TILE_TYPE_DIAG_SER:
					verts[0] = cpvmult(cpvadd(verts[0],verts[3]), 0.5);
					break;
				case TILE_TYPE_DIAG_SWR:
					verts[1] = verts[2];
					verts[2] = verts[3];
					verts[0] = cpvmult(cpvadd(verts[0],verts[2]), 0.5);
					len = 3;
					break;
				case TILE_TYPE_DIAG_SWL:
					verts[1] = cpvmult(cpvadd(verts[1],verts[2]), 0.5);
					break;
				case TILE_TYPE_DIAG_SE:
					verts[0] = verts[1];
					/* NO BREAK */
				case TILE_TYPE_DIAG_SW:
					verts[1] = verts[2];
					verts[2] = verts[3];
					len = 3;
					break;
				case TILE_TYPE_DIAG_NE:
					len = 3;
					break;
				case TILE_TYPE_DIAG_NW:
					verts[2] = verts[3];
					len = 3;
					break;
				}
				cpShape *shape = cpPolyShapeNew(current_space->staticBody, len, verts, cpvzero);
				cpSpaceAddStaticShape(current_space, shape);
				cpShapeSetFriction(shape, 0.9f);
				cpShapeSetElasticity(shape, 0.7f);

				meta_tile *meta = &lvl->tm.metadata[y][x];
				if (tilemap_isdestroyable(&lvl->tm, TLAY_SOLID, x, y)) {
					cpShapeSetCollisionType(shape, ID_GROUND_DESTROYABLE);
					meta->destroyable = WE_TRUE;
					meta->hp = 100;
				} else {
					cpShapeSetCollisionType(shape, ID_GROUND);
					meta->destroyable = WE_FALSE;
				}
				meta->x_col = x;
				meta->y_row = y;
				meta->block = shape;
				cpShapeSetUserData(shape, meta);
			}
			fprintf(stderr, "%c", tile ? '#' : '.');
		}
		fprintf(stderr, "%c", '\n');
	}

	float r_in = lvl->inner_radius;
	float r_out = lvl->outer_radius;
	float r_ceil = r_in;// + 64 * (r_out - r_in) / currentlvl->height;
	float r_floor = r_out;// - 5 * 64 * (r_out - r_in) / currentlvl->height;
	static const int segments = 100;
	static const float seg_radius = 50;
	static const float seg_length = 300;
	int i;
	for (i = 0; i < segments; ++i) {
		cpVect angle = cpvforangle(2 * M_PI * i / segments);
		cpVect n = cpvmult(cpvperp(angle), seg_length);
		cpVect p = cpvmult(angle, r_floor + seg_radius);

		cpVect a = cpvadd(p,cpvneg(n));
		cpVect b = cpvadd(p,n);

		cpShape *seg = cpSpaceAddShape(current_space, cpSegmentShapeNew(staticBody, a, b, seg_radius)); // ground level at 0
		cpShapeSetFriction(seg, 0.9f);
		cpShapeSetCollisionType(seg, ID_GROUND);
		cpShapeSetElasticity(seg, 0.7f);

		llist_add(lvl->ll_tileshapes, seg);
	}
	lvl->ceiling = cpSpaceAddShape(current_space, cpCircleShapeNew(staticBody, r_ceil, cpvzero));
	cpShapeSetFriction(lvl->ceiling, 0.9f);
	cpShapeSetCollisionType(lvl->ceiling, ID_GROUND);
	cpShapeSetElasticity(lvl->ceiling, 0.7f);

	lvl->loaded2space = current_space;

	return WE_TRUE;
}

we_bool spacelvl_unload2state(spacelvl *lvl)
{
	if (!lvl) {
		fprintf(stderr, "ERROR: trying to unload NULL spacelvl\n");
		return WE_FALSE;
	}
	if (lvl->loaded2space != current_space) {
		fprintf(stderr, "ERROR: trying to unload spacelvl with incorrect cpSpace!\n");
		return WE_FALSE;
	}

	objectsystem_clear();
	particles_clear(current_particles);

	/* remove old tilemap tiles */
	int x, y;
	for (y = lvl->tm.grid->pol.inner_i; y < lvl->tm.grid->pol.outer_i; y++) {
		for (x = 0; x < lvl->tm.grid->pol.cols; x++) {
			lvl->tm.metadata[y][x].block = NULL;
			cpShape *block = lvl->tm.metadata[y][x].block;
			if (block) {
				if(cpSpaceContainsShape(current_space, block)) {
					cpSpaceRemoveStaticShape(current_space, block);
					cpShapeFree(block);
				}
			}
		}

	}

	/* remove floor and ceiling */
	if (lvl->ceiling != NULL) {
		remove_static(lvl->ceiling);
		lvl->ceiling = NULL;
	}
	llist_clear(lvl->ll_tileshapes);

	lvl->loaded2space = NULL;
	return WE_TRUE;
}


void spacelvl_free(spacelvl *slvl)
{
	llist_destroy(slvl->ll_recipes);
	llist_destroy(slvl->ll_tileshapes);
	grid_free(slvl->tm.grid);
	free(slvl);

}
