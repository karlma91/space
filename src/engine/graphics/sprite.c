/*
 * sprite.c
 *
 *  Created on: 4. juli 2013
 *      Author: Mathias
 */

#include "sprite.h"
#include "../data/llist.h"
#include "../engine.h"
#include "texture.h"
#include "../io/waffle_utils.h"

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
		ret = sscanf(&buffer[counter], "%s %s %f %f %f %f %d%n\n", name, file_name, &u, &v, &width, &height, &subimages, &count);
		counter += count;
		if(ret==-1){
			break;
		}
		if(ret != 7){
			SDL_Log("SPRITE: file format error. Expected 7 arguments got: %d\n",ret);
			exit(5);
		}
		SDL_Log("SPRITE: %s %s %f %f %f %f %d", name, file_name, u, v, width, height, subimages);
		sprite_data *data = calloc(1,sizeof *data);
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
			llist_end_loop(constant_sprites); // <- missing end loop!
			return (SPRITE_ID)spr;
		}
	}
	SDL_Log("SPRITE: %s does not exist",name);
	llist_end_loop(constant_sprites);
	return NULL;
}

void sprite_destroy()
{
	llist_destroy(constant_sprites);
	//TODO go through and release all sprite data
}


void sprite_create(sprite *spr, SPRITE_ID id, int width, int height, float speed)
{
	memset(spr, 0, sizeof *spr);
	spr->id = id;
	spr->width = width;
	spr->height=height;
	spr->animation_speed = speed;
	spr->sub_index = 0;
}


void sprite_update(sprite *spr)
{
	if (!spr || !spr->id) return;

	sprite_data *data = (sprite_data*)spr->id;
	spr->sub_index += spr->animation_speed*dt;
	if (spr->sub_index >= data->subimages){
		spr->sub_index -= data->subimages;
		if (spr->sub_index >= data->subimages){
			spr->sub_index = 0;
		}
	} else if (spr->sub_index < 0){
		spr->sub_index -= data->subimages;
		if (spr->sub_index < 0){
			spr->sub_index = 0;
		}
	}
}

void sprite_get_current_image(sprite *spr, float *sub_map)
{
	if (!spr || !spr->id) return;

	sprite_data *data = (sprite_data*)spr->id;
	int index = floor(spr->sub_index);

	int sprites_x = 1/data->width;

	int index_x = index % sprites_x;
	int index_y = index / sprites_x;

	float tx_1 = data->u + index_x * data->width;
	float tx_2 = data->u + (index_x + 1) * data->width;
	float ty_2 = data->v + (index_y * data->height);
	float ty_1 = data->v + (index_y + 1) * data->height;

	sub_map[0] = tx_1;
	sub_map[1] = ty_1;
	sub_map[2] = tx_2;
	sub_map[3] = ty_1;
	sub_map[4] = tx_1;
	sub_map[5] = ty_2;
	sub_map[6] = tx_2;
	sub_map[7] = ty_2;
}

void sprite_set_length(sprite *spr, float length)
{
	spr->animation_speed = ((sprite_data*)spr->id)->subimages/length;
}

int sprite_get_texture(sprite *spr)
{
	return ((sprite_data*)spr->id)->tex_id;
}

void sprite_set_sprite_id(sprite *spr, SPRITE_ID id)
{
	spr->id = id;
}

void sprite_set_index(sprite *spr, int index)
{
	if (spr->id)
		spr->sub_index = index >= ((sprite_data *)spr->id)->subimages ? 0 : index < 0 ? 0 : index;
}

void sprite_set_index_normalized(sprite *spr, float p)
{
	if (spr->id) {
		spr->sub_index = (p > 1 ? 1 : (p < 0 ? 0 : p)) * (((sprite_data *)spr->id)->subimages-1);
	}
}

void sprite_render(sprite *spr, cpVect *pos, float angle)
{
	sprite_render_scaled(spr, pos, angle, 1);
}

void sprite_render_scaled(sprite *spr, cpVect *pos, float angle, float size)
{
	if (!spr) return;

	int tex_id = 0;
	sprite_data *data = (sprite_data*)spr->id;
	if (data) {
		tex_id = data->tex_id;
	}

	float sub_map[8];
	sprite_get_current_image(spr, sub_map);
	draw_texture(tex_id, pos, sub_map, spr->width*size, spr->height*size, angle);
}

