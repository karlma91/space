/*
 * sound.c
 *
 *  Created on: 6. juli 2013
 *      Author: Mathias
 */
#include <stdlib.h>
#include "sound.h"

#if SOUND_DISABLED
void sound_init(void) {}
Mix_Music *sound_loadmusic(const char *name) {return NULL;}
Mix_Chunk *sound_loadchunk(const char *name) {return NULL;}
void sound_play(Mix_Chunk *chunk) {}
void sound_music(Mix_Music *music) {}
void sound_destroy(void) {}
void sound_mute(void) {}
void sound_unmute(void) {}
void sound_music_mute(void) {}
void sound_music_unmute(void) {}
#else

#include "SDL_log.h"
#include "String.h"
#include "../data/hashmap.h"

#if __IPHONEOS__
#define SOUND_PATH ""
#else
#define SOUND_PATH "game_data/sound/"
#endif

#define SOUND_DEFAULT_VOLUME (SDL_MIX_MAXVOLUME/8)
#define SOUND_DEFAULT_MUSIC_VOLUME (SDL_MIX_MAXVOLUME/2)
#define NUM_CHANNELS 128
#define NUM_CHANNELS_PRESERVED 16

static hashmap *hm_tracks;
static hashmap *hm_chunks;

void sound_init(void)
{
	if (!Mix_Init(MIX_INIT_OGG)) {
		SDL_Log("ERROR: Mixer could not be initialized!");
		exit(-1);
	}

	hm_tracks = hm_create();
	hm_chunks = hm_create();
	//TODO remember to remove/free chunks/music

	if (Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,256)<0) {
		SDL_Log("ERROR: Opening mixer failed");
		exit(-1);
	}

	Mix_VolumeMusic(SOUND_DEFAULT_MUSIC_VOLUME);

	Mix_AllocateChannels(NUM_CHANNELS);
	Mix_ReserveChannels(NUM_CHANNELS_PRESERVED);
}

Mix_Music *sound_loadmusic(const char *name)
{
	char path[100] = SOUND_PATH;
	strcat(path, name);

	Mix_Music *track = hm_get(hm_tracks, name);
	if (track) {
		return track;
	}

	//waffle_read_file(sou) //TODO use waffle_read to read from zip !!!!!!!!!!!!!!!!!!!!!
	track = Mix_LoadMUS(path); // "Broken_Logic.ogg"
	if (track) {
		hm_add(hm_tracks, name, track);
	} else {
		SDL_Log("ERROR: Could not load soundtrack '%s'", name);
	}

	return track;
}

Mix_Chunk *sound_loadchunk(const char *name)
{
	Mix_Chunk *chunk = hm_get(hm_chunks, name);
	if (chunk) return chunk;

	char path[100] = SOUND_PATH;
	strcat(path, name);

	//waffle_read_file(sou) //TODO use waffle_read to read from zip
	chunk = Mix_LoadWAV(path);

	if (chunk) {
		Mix_VolumeChunk(chunk, SOUND_DEFAULT_VOLUME);
		hm_add(hm_chunks, name, chunk);
	} else {
		SDL_Log("ERROR: Could not load soundchunk '%s'", name);
	}

	return chunk;
}


void sound_play(Mix_Chunk *chunk)
{
	extern int objsys_terminating;
	if (!objsys_terminating)
		Mix_PlayChannel(-1, chunk,0);
}

void sound_music(Mix_Music *music)
{
	extern int objsys_terminating;
	if (!objsys_terminating) {
		Mix_PlayMusic(music, 1);
		if (Mix_PlayingMusic() == -1) {
			SDL_Log("ERROR: Music failed to played!");
		}
	}
}

void sound_destroy(void)
{
	//TODO iterate hashmaps and free chunks and music
	hm_destroy(hm_chunks);
	hm_destroy(hm_tracks);

	Mix_Quit();
}


void sound_mute(void)
{
	//Mix_HaltChannel(-1);
	Mix_Pause(-1);
	Mix_Volume(-1, 0);
}

void sound_unmute(void)
{
	Mix_Resume(-1);
	Mix_Volume(-1, SDL_MIX_MAXVOLUME);
}

void sound_music_mute(void)
{
	Mix_VolumeMusic(0);
	Mix_PauseMusic();
}

void sound_music_unmute(void)
{
	Mix_VolumeMusic(SOUND_DEFAULT_MUSIC_VOLUME);
	Mix_ResumeMusic();
}
#endif
