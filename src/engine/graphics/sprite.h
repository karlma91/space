/*
 * sprite.h
 *
 *  Created on: 4. juli 2013
 *      Author: Mathias
 *      Author: Karl
 */

#ifndef SPRITE_H_
#define SPRITE_H_

#include "chipmunk.h"
#include "we_utils.h"

typedef void * SPRITE_ID; /* Peker til konstant sprite_data (ikke sprite!)*/

/* public instance sprite */
typedef struct {
	SPRITE_ID id;
	//TODO legge til alle variabler som trengs og lagres individuelt for hver kopi av sprite
	float animation_speed; /* fps */
	float sub_index;
	float width;
	float height;
} sprite;

void sprite_init(void);
void sprite_destroy(void);

/*
 * sprite_load brukes for å koble og loade en sprite-fil til en SPRITE_ID, f.eks:
 *
 * //game_foo.h
 * extern int SPRITE_BAR;
 * //game_foo.c
 * foo_init(void) {
 * 	SPRITE_BAR = sprite_load("bar.sprite");
 * }
 */
SPRITE_ID sprite_load(const char *sprite); /* for loading av sprite data */

/*
 * allokerer og returnerer en ny enhet av sprite. Kalleren er selv ansvarlig for å frigjøre sprite (eller evt. kall sprite_free)
 */
#define SPRITE(NAME) SPRITE_##name
#define SPRITE_LINK(NAME) SPRITE(NAME) = sprite_link(#NAME);
#define SPRITE_DEF(NAME) SPRITE_ID SPRITE_##NAME
#define SPRITE_DEC(NAME) extern SPRITE_DEF(NAME)

#define REGISTER_SPRITE( NAME ) SPRITE_##NAME = sprite_link(#NAME);
SPRITE_ID sprite_link(const char *name);
void sprite_get_current_image(sprite *spr, float *sub_map);
int sprite_get_texture(sprite *spr);
void sprite_create(sprite *spr, SPRITE_ID id, int width, int height, float speed);

void sprite_set_size(sprite *spr, int width, int height);
void sprite_set_length(sprite *spr, float length);
void sprite_set_index(sprite *spr, int index);
void sprite_set_index_normalized(sprite *spr, float p);

void sprite_update(sprite *spr);
void sprite_render(int layer, sprite *spr, cpVect pos, float angle);
void sprite_render_body(int layer, sprite *spr, cpBody *body);
void sprite_render_scaled(int layer, sprite *spr, cpVect pos, float angle, float size);

#endif /* SPRITE_H_ */
