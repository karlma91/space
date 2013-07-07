/*
 * sound.c
 *
 *  Created on: 6. juli 2013
 *      Author: Mathias
 */

#include "SDL_mixer.h"

#if __IPHONEOS__
#define SOUND_PATH ""
#else
#define SOUND_PATH "sound/"
#endif

static Mix_Music * music;
static Mix_Chunk * chunk;

void sound_init()
{

	Mix_Init(MIX_INIT_OGG);


	if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,4096)<0)
		exit(-1);

	SDL_Log("DEBU: sound_init");
	music = Mix_LoadMUS(SOUND_PATH"Broken_Logic.ogg");

	chunk = Mix_LoadWAV(SOUND_PATH"laser_01.wav");

	if (!music) {
		SDL_Log("music not found");
		exit(1);
		return;
	}

	Mix_PlayMusic(music,1);

	if (Mix_PlayingMusic() == -1) {
		SDL_Log("music found, but not playing!");
	}

	Mix_VolumeMusic(SDL_MIX_MAXVOLUME);
}

void sound_play()
{
	Mix_PlayChannel(-1, chunk,0);
}

void sound_destroy()
{
	Mix_FreeChunk(chunk);
	Mix_FreeMusic(music);
	Mix_Quit();
}

