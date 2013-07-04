/*
 * sprite.c
 *
 *  Created on: 4. juli 2013
 *      Author: Mathias
 */

#include "sprite.h"
#include "../lib/LList/llist.h"
#include "texture.h"

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

void sprite_init()
{
	constant_sprites = llist_create();
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


void sprite_update(sprite spr)
{

}

void sprite_render(sprite spr)
{

}
