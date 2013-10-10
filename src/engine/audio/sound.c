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
Mix_Music *sound_load_music(const char *name) {return NULL;}
Mix_Chunk *sound_load_chunk(const char *name) {return NULL;}
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
#include "../data/llist.h"

#if __IPHONEOS__
#define SOUND_PATH ""
#else
#define SOUND_PATH "game_data/sound/"
#endif

#define SOUND_DEFAULT_VOLUME (SDL_MIX_MAXVOLUME/8)
#define SOUND_DEFAULT_MUSIC_VOLUME (SDL_MIX_MAXVOLUME/2)
#define NUM_CHANNELS 32

typedef struct {
	char name[100];
	int is_track;
	union {
		Mix_Music *track;
		Mix_Chunk *sound;
	};
} audio;

static LList tracks;
static LList chunks;

static void remove_audio(audio * a)
{
	if (a) {
		if (a->is_track) {
			Mix_FreeMusic(a->track);
		} else {
			Mix_FreeChunk(a->sound);
		}
		free(a);
	}
}

void sound_init(void)
{
	if (!Mix_Init(MIX_INIT_OGG)) {
		SDL_Log("ERROR: Mixer could not be initialized!");
		exit(-1);
	}

	tracks = llist_create();
	chunks = llist_create();

	llist_set_remove_callback(tracks, (ll_remove_callback) remove_audio);
	llist_set_remove_callback(chunks, (ll_remove_callback) remove_audio);

	if (Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048)<0) {
		SDL_Log("ERROR: Opening mixer failed");
		exit(-1);
	}

	Mix_VolumeMusic(SOUND_DEFAULT_MUSIC_VOLUME);

	Mix_AllocateChannels(NUM_CHANNELS);
}

Mix_Music *sound_load_music(const char *name)
{
	char path[100] = SOUND_PATH;
	strcat(path, name);

	Mix_Music *track = Mix_LoadMUS(path); // "Broken_Logic.ogg"

	if (track) {
		audio *a = malloc(sizeof(*a));
		strcpy(a->name, name);
		a->is_track = 0;
		a->track = track;

		llist_add(tracks, a);
	} else {
		SDL_Log("ERROR: Could not load soundtrack '%s'", name);
	}

	return track;
}

Mix_Chunk *sound_load_chunk(const char *name)
{
	char path[100] = SOUND_PATH;
	strcat(path, name);

	Mix_Chunk *chunk = Mix_LoadWAV(path); // "laser_02.ogg"

	if (chunk) {
		audio *a = malloc(sizeof(*a));
		strcpy(a->name, name);
		a->is_track = 0;
		a->sound = chunk;

		Mix_VolumeChunk(chunk, SOUND_DEFAULT_VOLUME);

		llist_add(tracks, a);
	} else {
		SDL_Log("ERROR: Could not load soundchunk '%s'", name);
	}

	return chunk;
}


void sound_play(Mix_Chunk *chunk)
{
	Mix_PlayChannel(-1, chunk,0);
}

void sound_music(Mix_Music *music)
{
	Mix_PlayMusic(music, 1);

	if (Mix_PlayingMusic() == -1) {
		SDL_Log("ERROR: Music failed to played!");
	}
}

void sound_destroy(void)
{
	llist_destroy(chunks);
	llist_destroy(tracks);

	Mix_Quit();
}


void sound_mute(void)
{
	Mix_HaltChannel(-1);
	Mix_Volume(-1, 0);
}

void sound_unmute(void)
{
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
