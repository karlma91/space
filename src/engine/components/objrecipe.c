#include "objrecipe.h"
#include "../io/jparse.h"

static void * get_param_direct(param_list *params, char *type, char * name)
{
	hashmap *names = (hashmap*)hm_get(params->param,type);

	if(names) {
		void *data = hm_get(names, name);
		if(data){
			return data;
		}else{
			//SDL_Log("LEVEL: could not find %s of type %s", name, type);
		}
	} else {
		//SDL_Log("LEVEL: Could not find type %s", type);
	}
	return NULL;
}


static void * get_param(param_list *params, const char *type, const char * name)
{
	char l_type[40];
	char l_name[40];

	strtolower(l_type, type);
	strtolower(l_name, name);
	void * param = get_param_direct(params, l_type, l_name);
	if (param) return param;

	//SDL_Log("LEVEL: Loading from param_DEFS");
	param = get_param_direct(params, l_type, l_name);
	if (param) return param;

	param = get_param_direct(params, l_type, "def");
	if (param) return param;

	//SDL_Log("LEVEL: Could not find param: %s for type: %s", l_name, l_type);
	return NULL;
}

static void add_object_recipe_name(LList * l, param_list *pl, const char * obj_type, const char * param_name, cpVect pos, float rotation)
{

	if(!obj_type){
		SDL_Log("LEVEL: INVALID OBJECT TYPE");
		return;
	}

	if(!param_name){
		SDL_Log("LEVEL: INVALID param NAME");
		return;
	}

	char data_type[32];
	char data_name[32];

	strtolower(data_type, obj_type);
	strtolower(data_name, param_name);

	SDL_Log("LEVEL: ADDING %s,  %s", data_type, data_name);

	object_recipe * rec = calloc(1, sizeof(object_recipe));
	rec->obj_type = object_by_name(data_type);
	rec->param = get_param(pl, data_type, data_name);
	strcpy(rec->param_name, data_name);
	rec->pos = pos;
	rec->rotation = rotation;
	llist_set_remove_callback(l, free);
	llist_add(l, rec);
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

}

void objrecipe_load_objects(LList l, cJSON* object_array, param_list *pl)
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
		}
		add_object_recipe_name(l, pl, type, name, p, 0);
	}
}


void objrecipe_init()
{
	jparse_register("objrecipe",objrecipe_parse);
}
