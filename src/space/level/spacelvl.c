#include "we.h"
#include "spacelvl.h"

#define FILE_SIZE_BUFFER 128000

we_bool spacelvl_init(void)
{
	srand(0x9b3a09fa);
	return 1;
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
	llist_set_remove_callback(lvl->ll_recipes, free);

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

we_bool spacelvl_copy(spacelvl *lvl_dst, spacelvl *lvl_src)
{
	return WE_FALSE;
}

we_bool spacelvl_load2state(spacelvl *lvl)
{
	cpResetShapeIdCounter();
	llist_begin_loop(lvl->ll_recipes);
	while(llist_hasnext(lvl->ll_recipes)) {
		object_recipe * data = llist_next(lvl->ll_recipes);
		SDL_Log("INSTANTIATING TYPE: %s at: %f ", data->obj_type->NAME, data->pos.x);
		instance_create(data->obj_type, data->param, data->pos, cpvzero);
	}
	llist_end_loop(lvl->ll_recipes);

	//TODO add floor and ceiling and init tilemap shapes here...
	return WE_TRUE;
}

we_bool spacelvl_unload2state(spacelvl *lvl)
{
	//TODO clear cpspace and objectsystem

	//TODO add floor and ceiling and init tilemap shapes here...
	return WE_FALSE;
}

void spacelvl_destroy(void)
{

}
