/*
 * sprite.c
 *
 *  Created on: 4. juli 2013
 *      Author: Mathias
 */

#include "sprite.h"
#include "../lib/LList/llist.h"
#include "texture.h"
#include "main.h"
#include "waffle_utils.h"

LList constant_sprites;

typedef struct {
	char name[50];
	int tex_id;
	float u;
	float v;
	float width;
	float height;
	int subimages;
} sprite_data;

#define FILE_BUFFER_SIZE 8192 /* NB! make sure buffer is large enough */
static char buffer[FILE_BUFFER_SIZE];

void sprite_init()
{

	SDL_Log("SPRITE INTIT:");
	constant_sprites = llist_create();
	llist_set_remove_callback(constant_sprites,free);

	char name[100];
	char file_name[100];
	float u,v,width,height;
	int subimages, counter=0, count=0;


	/* read space station data */
	int filesize = waffle_read_file("textures/sprites.csv", &buffer[0], FILE_BUFFER_SIZE);
	if (!filesize) {
		SDL_Log("Could not load SPRITE data!");
		exit(1);
	}



	int ret = 0;
	while(counter <= filesize){
		ret = sscanf(buffer, "%s %s %f %f %f %f %d %n\n", name, file_name, &u, &v, &width, &height, &subimages, &count);
		counter += count;
		if(ret != 7){
			SDL_Log("SPRITE: file format error. Expected 7 arguments got: %d\n",ret);
			exit(5);
		}
		SDL_Log("SPRITE: %s %s %f %f %f %d", name, file_name, u, v, width, height, subimages);
		sprite_data *data = (sprite_data*)malloc(sizeof(sprite_data));
		strcpy(data->name, name);
		data->height = height;
		data->width = width;
		data->u = u;
		data->v = v;
		data->subimages = subimages;
		data->tex_id = texture_load(file_name);
		llist_add(constant_sprites,data);
	}
}

SPRITE_ID sprite_link(const char *name)
{
	llist_begin_loop(constant_sprites);
	while(llist_hasnext(constant_sprites)){
		sprite_data *spr = llist_next(constant_sprites);
		if(strcmp(spr->name, name) == 0){
			return (SPRITE_ID)spr;
		}
	}
	llist_end_loop(constant_sprites);
	return NULL;
}

void sprite_destroy()
{
	llist_destroy(constant_sprites);
	//TODO go through and release all sprite data
}


sprite *sprite_create(SPRITE_ID id)// ,posisjon, etc..)
{

	return NULL;
}


void sprite_update(sprite *spr)
{
	sprite_data *data = (sprite_data*)spr->id;
	spr->sub_index += spr->animation_speed*dt;
	if(spr->sub_index >= data->subimages){
		spr->sub_index -= data->subimages;
	}
}

void sprite_render(sprite *spr, cpVect *pos, float angle)
{
	sprite_data *data = (sprite_data*)spr->id;
	int index = floor(spr->sub_index);

	int sprites_x = 1/data->width;
	int sprites_y = 1/data->height;

	int index_x = index % sprites_x;
	int index_y = index / sprites_y;

	float tx_1 = data->u + index_x * data->width;
	float tx_2 = data->u + (index_x + 1) * data->width;
	float ty_2 = data->v + (index_y * data->height);
	float ty_1 = data->v + (index_y + 1) * data->height;

	float sub_map[8] = {
			tx_1, ty_1,
			tx_2, ty_1,
			tx_1, ty_2,
			tx_2, ty_2,
	};
	draw_texture(data->tex_id, pos, sub_map, spr->width, spr->height, angle);
}
