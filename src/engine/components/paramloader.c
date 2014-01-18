/*
 * paramloader.c
 *
 *  Created on: Jan 14, 2014
 *      Author: karlmka
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "paramloader.h"
#include "we_data.h"

hashmap *parse_functions;

/**
 * Parse an int from cJSON struct
 */
static we_bool parse_int(cJSON *t, int *field, int *def)
{
	if(t){
		*field = t->valueint;
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool parse_float(cJSON *t, float *field, float *def)
{
	if(t){
		*field = t->valuedouble;
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool parse_char(cJSON *t, char *field, char *def)
{
	if(t){
		strcpy(field, t->valuestring);
		return WE_TRUE;
	}else{
		strcpy(field, def);
		return WE_FALSE;
	}
}

static we_bool parse_object_id(cJSON *t, object_id **field, object_id **def)
{
	if(t){
		*field = object_by_name(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

we_bool parse_shape(cJSON *t, polyshape *field,  polyshape *def)
{
	if(t) {
		*field = shape_read(t->valuestring);
		return WE_TRUE;
	} else {
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool parse_sprite(cJSON *t, SPRITE_ID *field, SPRITE_ID *def)
{
	if(t){
		*field = sprite_link(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool parse_emitter(cJSON *t, EMITTER_ID *field, EMITTER_ID *def)
{
	if(t){
		*field = particles_bind_emitter(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool parse_sound(cJSON *t, Mix_Chunk **field, Mix_Chunk **def)
{
	if(t){
		*field = sound_loadchunk(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}

static we_bool parse_texture(cJSON *t, int *field, int *def)
{
	if(t){
		*field = texture_load(t->valuestring);
		return WE_TRUE;
	}else{
		*field = *def;
		return WE_FALSE;
	}
}
static we_bool parse_color(cJSON *t, Color *field, Color *def)
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

void pl_init(void)
{
	parse_functions = hm_create();
	pl_register_type("int", (void*)parse_int);
	pl_register_type("float", (void*)parse_float);
	pl_register_type("char", (void*)parse_char);
	pl_register_type("sprite", (void*)parse_sprite);
	pl_register_type("emitter", (void*)parse_emitter);
	pl_register_type("sound", (void*)parse_sound);
	pl_register_type("texture", (void*)parse_texture);
	pl_register_type("object_id", (void*)parse_object_id);
	pl_register_type("shape", (void*)parse_shape);
	pl_register_type("Color", (void*)parse_color);
}

void pl_register_type(char *type, parse_function pf)
{
	hm_add(parse_functions, type, pf);
	SDL_Log("PARAMLOADER: Added parsefunction %s", type);
}

we_bool pl_parse(cJSON *param, char *field_name, char *type, void *field, void *def)
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
