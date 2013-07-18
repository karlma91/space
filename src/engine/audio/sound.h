/*
 * sound.h
 *
 *  Created on: 6. juli 2013
 *      Author: Mathias
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "SDL_mixer.h"

void sound_init();

Mix_Music *sound_load_music(const char *name);
Mix_Chunk *sound_load_chunk(const char *name);

void sound_play(Mix_Chunk *chunk);
void sound_music(Mix_Music *music);

void sound_mute();
void sound_unmute();

void sound_music_mute();
void sound_music_unmute();

void sound_destroy();

#endif /* SOUND_H_ */
