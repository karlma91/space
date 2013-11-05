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
#include "../state/statesystem.h"

typedef struct {
	char name[SPRITE_MAX_LEN];
	int tex_id;
	array *sub_images; /* <sprite_subimg> */
} sprite_data;

#define FILE_BUFFER_SIZE (1024*256)
static char buffer[FILE_BUFFER_SIZE];

hashmap *hm_sprites;


static int chr_unread, line_size;
static char *cur, *next;

static inline void *readln(void)
{
	cur = next;
	next = memchr(cur, '\n', chr_unread);
	if (next) {
		*next++ = '\0';
		line_size = (int)(next - cur);
		chr_unread -= line_size;
	} else {
		line_size = chr_unread;
		cur[chr_unread] = '\0';
		chr_unread = 0;
	}
	return cur;
}


void sprite_init(void)
{
	hm_sprites = hm_create();

	/* read texture package file */
	int filesize = waffle_read_file("textures/spacetex.pack", &buffer[0], FILE_BUFFER_SIZE);
	if (!filesize) {
		SDL_Log("ERROR: Could not load texpack data!");
		exit(1);
	}

	chr_unread = filesize;
	next = buffer;

	readln();
	char *texname = readln();
	readln(); //format: format
	readln(); //filter: Mag,Min
	readln(); //repeat: x/y/xy

	char *suffix = memchr(texname, '.', line_size);
	if (strcmp(suffix, ".png") == 0) {
		strcpy(suffix, ".pvr");
	}

	//TODO get texture id
	int tex_id = texture_load(texname);

	int i;
	float x, y, w, h;
	while(chr_unread > 0) {
		char *sprname = readln();           // "sprname"
		readln();                           // "  rotate: true/false"
		char *pos  = readln();    sscanf(pos,  "  xy: %f, %f", &x, &y);
		char *size = readln();    sscanf(size, "  size: %f, %f", &w, &h);
		readln();                           // "  orig: w, h"
		readln();                           // "  offset: x, y"
		char *index = readln();   sscanf(index,"  index: %d", &i);

		//TODO normalize uv-pos and size

		sprite_subimg subimg = {x,y,x+w,y+h};
		subimg = texture_normalize_uv(tex_id, subimg);
		sprite_add_subimg(tex_id, sprname, subimg, i);
	}
}

void strtolower(char *to, const char *from)
{
	int i;
	for (i=0; from[i]; i++) {
		to[i] = tolower(from[i]);
	}
	to[i] = '\0';
}

SPRITE_ID sprite_add_subimg(int tex_id, const char *spr_name, sprite_subimg subimg, int index)
{
	sprite_data *spr_id = hm_get(hm_sprites, spr_name);
	if (!spr_id) {
		spr_id = (sprite_data *) calloc(1, sizeof(sprite_data));
		strtolower(spr_id->name, spr_name);
		spr_id->tex_id = tex_id;
		spr_id->sub_images = array_new(sizeof(sprite_subimg));
		hm_add(hm_sprites, spr_id->name, spr_id);
	}
	index = index < 1 ? 0 : index - 1; // NB! Forces external index to start at 1 (for easier Blender import (blender starts at frame 1 as default))
	array_set_safe(spr_id->sub_images, index, &subimg);
	fprintf(stderr, "SPRITE: %16s[%d] = {%f, %f, %f, %f}\n", spr_id->name, index, subimg.x1, subimg.y1, subimg.x2, subimg.y2);
	return spr_id;
}

SPRITE_ID sprite_link(const char *name)
{
	char name_lower[SPRITE_MAX_LEN];
	strtolower(name_lower, name);
	sprite_data *spr_id = hm_get(hm_sprites, name_lower);
	if (!spr_id) SDL_Log("SPRITE: %s does not exist",name_lower);
	return spr_id;
}

void sprite_destroy(void)
{
	//TODO go through and release all sprite data
	hm_destroy(hm_sprites);
}


void sprite_create(sprite *spr, SPRITE_ID id, int width, int height, float speed)
{
	memset(spr, 0, sizeof *spr);
	spr->id = id;
	sprite_set_size(spr, width, height);
	spr->animation_speed = speed;
	spr->sub_index = 0;
}

void sprite_set_size(sprite *spr, int width, int height) {
	spr->width = width;
	spr->height = height;
}


void sprite_update(sprite *spr)
{
	if (!spr || !spr->id) return;
	sprite_data *data = (sprite_data*)spr->id;
	int subimages = array_count(data->sub_images);
	spr->sub_index += spr->animation_speed*dt;
	if (spr->sub_index >= subimages){
		spr->sub_index -= subimages;
		if (spr->sub_index >= subimages){
			spr->sub_index = 0;
		}
	} else if (spr->sub_index < 0){
		spr->sub_index -= subimages;
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
	sprite_subimg *subimg = (sprite_subimg *)array_get(data->sub_images, index);

	float tx_1 = subimg->x1;
	float tx_2 = subimg->x2;
	float ty_2 = subimg->y1; // vertical flip
	float ty_1 = subimg->y2;

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
	sprite_data *data = (sprite_data*)spr->id;
	int subimages = array_count(data->sub_images);
	spr->animation_speed = subimages/length;
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
	if (spr->id) {
		sprite_data *data = (sprite_data*)spr->id;
		int subimages = array_count(data->sub_images);
		spr->sub_index = index >= subimages ? 0 : index < 0 ? 0 : index;
	}
}

void sprite_set_index_normalized(sprite *spr, float p)
{
	if (spr->id) {
		sprite_data *data = (sprite_data*)spr->id;
		int subimages = array_count(data->sub_images);
		spr->sub_index = (p > 1 ? 1 : (p < 0 ? 0 : p)) * (subimages-1);
	}
}

void sprite_render(int layer, sprite *spr, cpVect pos, float angle)
{
	sprite_render_scaled(layer, spr, pos, angle, 1);
}

void sprite_render_body(int layer, sprite *spr, cpBody *body)
{
	sprite_render_scaled(layer, spr, body->p, body->a, 1);
}

void sprite_render_scaled(int layer, sprite *spr, cpVect pos, float angle, float size)
{
	if (!spr) return;

	int tex_id = 0;
	sprite_data *data = (sprite_data*)spr->id;
	if (data) {
		tex_id = data->tex_id;
	}

	float sub_map[8];
	sprite_get_current_image(spr, sub_map);
	draw_texture(layer, tex_id, pos, sub_map, cpvmult(cpv(spr->width, spr->height), size), angle);
}

