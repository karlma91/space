#include "objrecipe.h"
#include "../io/jparse.h"

we_bool add_object_recipe_name(LList * ll_recipes, const char * obj_type, const char * param_name, cpVect pos, float rotation)
{
	if(!obj_type){
		SDL_Log("LEVEL: INVALID OBJECT TYPE");
		return WE_FALSE;
	}

	if(!param_name){
		SDL_Log("LEVEL: INVALID param NAME");
		return WE_FALSE;
	}

	char data_type[32];
	char data_name[32];

	strtolower(data_type, obj_type);
	strtolower(data_name, param_name);

	SDL_Log("LEVEL: ADDING %s,  %s", data_type, data_name);

	object_recipe * rec = calloc(1, sizeof(object_recipe));
	rec->obj_type = object_by_name(data_type);
	rec->param = param_get(data_type, data_name);
	strcpy(rec->param_name, data_name);
	rec->pos = pos;
	rec->rotation = rotation;
	llist_set_remove_callback(ll_recipes, free);
	llist_add(ll_recipes, rec);
	return WE_TRUE;
}

static we_bool objrecipe_parse(cJSON *t, object_recipe *objrep, void *def )
{
	char type[128], name[128];
	jparse_parse(t,"type", "char", type, "NULL");
	jparse_parse(t,"name", "char", name, "NULL");

	strtolower(type, type);
	strtolower(name, name);

	SDL_Log("ADDING OBJECT TYPE: %s name: %s ", type, name);
	cJSON *pos = cJSON_GetObjectItem(t,"pos");
	cpVect p = cpvzero;
	if(pos != NULL){
		float pos_def = 0;
		float x,y;
		//&(p.x) do not work because its a double
		jparse_parse(pos,"x","float",&(x),&pos_def);
		jparse_parse(pos,"y","float",&(y),&pos_def);
		p.x = x;
		p.y = y;
	} else {
		SDL_Log("LEVEL PARSING ERROR: Cannot find field pos in object");
		return WE_FALSE;
	}
	return WE_TRUE;
}

static we_bool objrecipe_write(cJSON *t,LList l, void *def )
{
	return WE_FALSE;
}

we_bool objrecipe_load_objects(LList l, cJSON* object_array)
{
	int i;
	for (i = 0; i < cJSON_GetArraySize(object_array); i++){
		cJSON *object = cJSON_GetArrayItem(object_array, i);
		char type[128], name[128];
		jparse_parse(object,"type", "char", type, "NULL");
		jparse_parse(object,"name", "char", name, "NULL");

		strtolower(type, type);
		strtolower(name, name);

		SDL_Log("ADDING OBJECT TYPE: %s name: %s ", type, name);
		cJSON *pos = cJSON_GetObjectItem(object,"pos");
		cpVect p = cpvzero;
		if(pos != NULL){
			float pos_def = 0;
			float x,y;
			//&(p.x) do not work because its a double
			jparse_parse(pos,"x","float",&(x),&pos_def);
			jparse_parse(pos,"y","float",&(y),&pos_def);
			p.x = x;
			p.y = y;
		} else {
			SDL_Log("LEVEL PARSING ERROR: Cannot find field pos in object");
			return WE_FALSE;
		}
		add_object_recipe_name(l, type, name, p, 0);
	}
	return WE_TRUE;
}

void objrecipe_free(object_recipe *l)
{
	free(l);
}
void objrecipe_init()
{
	jparse_register("objrecipe",objrecipe_parse);
}
