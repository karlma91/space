/*
 * sound.h
 *
 *  Created on: 6. juli 2013
 *      Author: Mathias
 */

#ifndef SOUND_H_
#define SOUND_H_

#ifndef SOUND_DISABLED
#define SOUND_DISABLED 1
#endif
#if SOUND_DISABLED
typedef void *Mix_Music;
typedef void *Mix_Chunk;
#else
#include "SDL_mixer.h"
#endif

void sound_init(void);

Mix_Music *sound_loadmusic(const char *name);
Mix_Chunk *sound_loadchunk(const char *name);

void sound_play(Mix_Chunk *chunk);
void sound_music(Mix_Music *music);

void sound_mute(void);
void sound_unmute(void);

void sound_music_mute(void);
void sound_music_unmute(void);

void sound_destroy(void);

#endif /* SOUND_H_ */
