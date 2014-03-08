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

#define SPRITE_MAX_LEN 50

typedef struct sprite_subimg {
	float x1,y1,x2,y2;
} sprite_subimg;

/* public instance sprite */
typedef struct {
	SPRITE_ID id;
	//TODO legge til alle variabler som trengs og lagres individuelt for hver kopi av sprite
	float animation_speed; /* fps */
	float sub_index;
	float width;
	float height;
	int antirot;
} sprite;

typedef struct sprite_ext {
	cpVect pos;
	float angle;
	sprite spr;
	Color col;
} sprite_ext;


/* sprites used by engine */
extern SPRITE_ID SPRITE_ERROR;
extern SPRITE_ID SPRITE_GLOWDOT;
extern SPRITE_ID SPRITE_DOT;
extern SPRITE_ID SPRITE_GLOW;
extern SPRITE_ID SPRITE_WHITE;
extern SPRITE_ID SPRITE_BAR;


void sprite_init(void);
int sprite_packload(const char* spritepack); /* returns 0 on success and -1 on failure */
void sprite_destroy(void);

SPRITE_ID sprite_add_subimg(int tex_id, const char *spr_name, sprite_subimg subimg, int index);

/*
 * allokerer og returnerer en ny enhet av sprite. Kalleren er selv ansvarlig for å frigjøre sprite (eller evt. kall sprite_free)
 */
#define SPRITE(NAME) SPRITE_##name
#define SPRITE_LINK(NAME) SPRITE(NAME) = sprite_link(#NAME);
#define SPRITE_DEF(NAME) SPRITE_ID SPRITE_##NAME
#define SPRITE_DEC(NAME) extern SPRITE_DEF(NAME)

#define REGISTER_SPRITE( NAME ) SPRITE_##NAME = sprite_link(#NAME);
SPRITE_ID sprite_link(const char *name);
float sprite_get_aspect_ratio(SPRITE_ID id); /* returns sprite_height/sprite_width */
void sprite_get_current_image(sprite *spr, float *sub_map);
void sprite_get_subimg_by_index(SPRITE_ID id, int index, float *sub_map);
void sprite_get_subimg_mapped(SPRITE_ID id, sprite_subimg subimg, float *sub_map);
void sprite_subimg2submap(sprite_subimg img, float *sub_map);
int sprite_get_texture(SPRITE_ID id);
void sprite_create(sprite *spr, SPRITE_ID id, int width, int height, float speed);
sprite_subimg sprite_get_subimg(SPRITE_ID id);

char * sprite_get_name(SPRITE_ID id);

void sprite_set_size(sprite *spr, int width, int height);
void sprite_set_length(sprite *spr, float length);
void sprite_set_index(sprite *spr, int index);
void sprite_set_index_normalized(sprite *spr, float p);

void sprite_update(sprite *spr);
void sprite_render_by_id(int layer, SPRITE_ID id, cpVect pos, cpVect size, float angle);
void sprite_render_index_by_id(int layer, SPRITE_ID id, int index, cpVect pos, cpVect size, float angle);
void sprite_render(int layer, sprite *spr, cpVect pos, float angle);
void sprite_render_body(int layer, sprite *spr, cpBody *body);
void sprite_render_scaled(int layer, sprite *spr, cpVect pos, float angle, float size);

#endif /* SPRITE_H_ */
