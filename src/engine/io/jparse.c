#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "we_utils.h"
#include "we_data.h"
#include "we_object.h"
#include "we_graphics.h"
#include "../audio/sound.h"
#include "jparse.h"

#define FILE_SIZE_BUFFER 128000

hashmap *parse_functions;

char buff[FILE_SIZE_BUFFER];

void jparse_close(cJSON* j)
{
	cJSON_Delete(j);
}

cJSON * jparse_open(int dir_type, char *filepath)
{
	cJSON *root;
	int filesize = waffle_read_file_zip("level/object_defaults.json", &buff[0], 10000);
	if (!filesize) {
		SDL_Log("Could not load level data!");
		return NULL;
	}
	root = cJSON_Parse(buff);
	if(root == NULL){
		SDL_Log("[Jparser] could not parse file: %s", filepath);
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
		return NULL;
	}
	return root;
}

cJSON* jparse_new()
{

}

void jparse_save(cJSON *c, int dir_type, char *filepath)
{

}


static we_bool jparse_int(cJSON *t, int *field, int *def)
{
	if(t){
		*field = t->valueint;
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool jparse_float(cJSON *t, float *field, float *def)
{
	if(t){
		*field = (float)t->valuedouble;
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool jparse_char(cJSON *t, char *field, char *def)
{
	if(t){
		strcpy(field, t->valuestring);
		return WE_TRUE;
	}else{
		strcpy(field, def);
		return WE_FALSE;
	}
}

static we_bool jparse_object_id(cJSON *t, object_id **field, object_id **def)
{
	if(t){
		*field = object_by_name(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

we_bool jparse_shape(cJSON *t, polyshape *field,  polyshape *def)
{
	if(t) {
		*field = shape_read(t->valuestring);
		return WE_TRUE;
	} else {
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool jparse_sprite(cJSON *t, SPRITE_ID *field, SPRITE_ID *def)
{
	if(t){
		*field = sprite_link(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool jparse_emitter(cJSON *t, EMITTER_ID *field, EMITTER_ID *def)
{
	if(t){
		*field = particles_bind_emitter(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool jparse_sound(cJSON *t, Mix_Chunk **field, Mix_Chunk **def)
{
	if(t){
		*field = sound_loadchunk(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool jparse_texture(cJSON *t, int *field, int *def)
{
	if(t){
		*field = texture_load(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}
static we_bool jparse_color(cJSON *t, Color *field, Color *def)
{
	if(t){
		int v;
		sscanf(t->valuestring,"%x",&v);
		field->r = (v>>24) & 0xff;
		field->g = (v>>16) & 0xff;
		field->b = (v>>8) & 0xff;
		field->a = (v>>0) & 0xff;
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

void jparse_init(void)
{
	parse_functions = hm_create();
	jparse_register("int", (void*)jparse_int);
	jparse_register("float", (void*)jparse_float);
	jparse_register("char", (void*)jparse_char);
	jparse_register("sprite", (void*)jparse_sprite);
	jparse_register("emitter", (void*)jparse_emitter);
	jparse_register("sound", (void*)jparse_sound);
	jparse_register("texture", (void*)jparse_texture);
	jparse_register("object_id", (void*)jparse_object_id);
	jparse_register("shape", (void*)jparse_shape);
	jparse_register("Color", (void*)jparse_color);
}

void jparse_register(char *type, parse_function pf)
{
	hm_add(parse_functions, type, pf);
	SDL_Log("PARAMLOADER: Added parsefunction %s", type);
}

we_bool jparse_parse(cJSON *param, char *field_name, char *type, void *field, void *def)
{
	cJSON *t = cJSON_GetObjectItem(param, field_name);
	parse_function pf = (parse_function)hm_get(parse_functions, type);
	if (pf != NULL) {
		if(!t && !def){
			SDL_Log("ERROR LOADING: %s, %s", field_name, type);
			we_error("ERROR LOADING DEFAULTS! fix object_defaults.json");
			return WE_FALSE;
		} else {
			if(!pf(t,field, def)){
				SDL_Log("ERROR PARSING: %s, %s", field_name, type);
				return WE_FALSE;
			}else{
				return WE_TRUE;
			}
		}
	} else {
		SDL_Log("PARAMLOADER: No type with name %s", type);
		return WE_FALSE;
	}
}
