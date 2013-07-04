/*
 * sprite.h
 *
 * En generell implementasjon av sprites.
 *
 * SPRITE_ID:
 * - konstant sprite data (slik som: texture, uv-koordinater, antall sub_images, origin, etc..)
 *
 * sprite:
 *  - id: Referanse til konstant sprite data
 *  - posisjonspeker / posisjon
 *  - vinkelpeker / vinkel
 *  - float sprite_index (sub_image index)
 *  - image/sprite_speed (animasjonshastighet)
 *
 *
 *  Created on: 4. juli 2013
 *      Author: Mathias
 * TODO: sprite_init
 * TODO: sprite_load
 * TODO: sprite_destroy
 *
 *      Author: Karl
 * TODO: automatisk animasjon basert på animasjonshastighet
 * TODO: manuell animasjon ved å sette sprite_index direkte og la sprite_speed = 0
 * TODO: resten
 */

#ifndef SPRITE_H_
#define SPRITE_H_

typedef void * SPRITE_ID; /* Peker til konstant sprite data*/

/* public instance sprite */
typedef struct {
	SPRITE_ID id;
	//TODO legge til alle variabler som trengs og lagres individuelt for hver kopi av sprite
} sprite;

void sprite_init();
void sprite_destroy();

/*
 * sprite_load brukes for å koble og loade en sprite-fil til en SPRITE_ID, f.eks:
 *
 * //game_foo.h
 * extern int SPRITE_BAR;
 * //game_foo.c
 * foo_init() {
 * 	SPRITE_BAR = sprite_load("bar.sprite");
 * }
 */
SPRITE_ID sprite_load(const char *sprite); /* for loading av sprite data */

/*
 * allokerer og returnerer en ny enhet av sprite. Kalleren er selv ansvarlig for å frigjøre sprite (eller evt. kall sprite_free)
 */
sprite *sprite_create(SPRITE_ID id);// ,posisjon, etc..);

void sprite_set_speed(sprite spr, float speed);

void sprite_update(sprite spr);
void sprite_render(sprite spr);

#endif /* SPRITE_H_ */
