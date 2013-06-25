#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "SDL.h"
#include "SDL_image.h"

#include "waffle_utils.h"

#if TARGET_OS_IPHONE | __ANDROID__
#define TEXTURE_RESOLUTION "_low"
#else
#define TEXTURE_RESOLUTION ""
#endif

#define MAX_IMAGE_BUFFER 524288 /* ( 0.5 MiB) NB! make sure it is enough RAM to hold buffer */

/**
 * texture values (GLOBAL)
 */
GLuint *textures;

const float TEX_MAP_FULL[8] = {0,1, 1,1, 0,0, 1,0};

/**
 * Texture names
 */
static char (*names)[51];

/**
 * number of textures
 */
static int tex_counter = -1;

static int texture_from_name(char *file);
static GLenum GL_ENUM_TYPE = GL_UNSIGNED_BYTE;


int texture_load(char *file)
{
#if !LOAD_TEXTURES
	return 0;
#endif

	int have_texture = texture_from_name(file);
	if(have_texture >=0){
		//SDL_Log("have texture: %s\n", file);
		return have_texture;
	}

	char filepath[64];
	sprintf(filepath,"textures/%s", file);

	/* append texture resolution suffix */
	filepath[strlen(filepath) - 4] = 0; // ignore .png suffix
	strcat(filepath,TEXTURE_RESOLUTION ".png");

	SDL_RWops *rw;

	char buffer[MAX_IMAGE_BUFFER];
	int filesize = waffle_read_file(filepath, buffer, MAX_IMAGE_BUFFER);
	if (!filesize) {
		SDL_Log("ERROR: Unable to find texture: %s", file);
	}

	rw = SDL_RWFromMem(&buffer[0], filesize);

	SDL_Surface* img = IMG_Load_RW(rw, 0);

	if (img) {
		unsigned int tex_id;
		int w, h;
		w = img->w;
		h = img->h;

		++tex_counter;
		names = realloc(names,(sizeof(char[tex_counter+1][51])));
		strcpy(names[tex_counter],file);
		textures = realloc(textures,sizeof(int[(tex_counter + 1)]));

		SDL_ConvertSurfaceFormat(img,SDL_PIXELFORMAT_RGBA8888,0);

		/*Generate an OpenGL 2D texture from the SDL_Surface*.*/
		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_2D, tex_id);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if (__MACOSX__ | __IPHONEOS__)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_ENUM_TYPE, img->pixels);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_ENUM_TYPE, img->pixels);
#endif
		SDL_FreeSurface(img);

		textures[tex_counter] = tex_id;

		SDL_Log("DEBUG: Texture loaded: %s\n", file);
		return tex_counter;

	} else {
		SDL_Log("DEBUG: Unable to load texture: %s\n IMG_ERROR: %s\n", filepath, IMG_GetError());
		return -1;
	}
}

static int texture_from_name(char *file)
{
	int i;
	for(i=0;i<=tex_counter; i++){
		if(strcmp(names[i],file) == 0){
			return i;
		}
	}
	return -1;
}

#include "SDL_endian.h"
int texture_init()
{
	TEX_CLOUD = texture_load("cloud.png");
	TEX_CLOUD_ULQ = texture_load("cloud_ultralow.png");
	TEX_CLOUD_LQ = texture_load("cloud_low.png");
	TEX_WHEEL   = texture_load("wheel.png");
	TEX_DOT     = texture_load("dot.png");
	TEX_GLOW    = texture_load("glow.png");
	TEX_GLOWDOT = texture_load("glowdot.png");
	TEX_PLAYER = texture_load("player.png");
	TEX_PARTICLE = texture_load("particle.png");
	TEX_BUTTON = texture_load("button.png");
	TEX_BUTTON_DOWN = texture_load("button_down.png");

	return 0;
}

int texture_destroy()
{
	return 0;
}

int texture_bind(int tex_id) {
#if !LOAD_TEXTURES
	return 0;
#endif
	static int last_tex_id = -1;
	if (tex_id != last_tex_id && tex_id >= 0) {
		last_tex_id = tex_id;
		glBindTexture(GL_TEXTURE_2D, textures[tex_id]);
		return 0;
	}
	return 1;
}
