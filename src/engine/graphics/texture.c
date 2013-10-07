#include <stdio.h>
#include <string.h>

#include "texture.h"
#include "SDL.h"
#include "SDL_image.h"

#include "../io/waffle_utils.h"
#include "draw.h"

#if __ANDROID__ // | TARGET_OS_IPHONE
#define TEXTURE_RESOLUTION "_low"
#else
#define TEXTURE_RESOLUTION ""
#endif

#define MAX_IMAGE_BUFFER 786432 /* ( 0.7 MiB) NB! make sure it is enough RAM to hold buffer */

int TEX_GLOW_DOT;
int TEX_GLOW;
int TEX_LIGHT;
int TEX_STARS;

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

static int texture_from_name(const char *file);
static GLenum GL_ENUM_TYPE = GL_UNSIGNED_BYTE;


int texture_load(const char *file)
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
	char *file_suffix = &file[strlen(file) - 4];

	filepath[strlen(filepath) - 4] = 0; // ignore file suffix
	strcat(filepath, TEXTURE_RESOLUTION);
	strcat(filepath, file_suffix);

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

		img = SDL_ConvertSurfaceFormat(img,SDL_PIXELFORMAT_ARGB8888,0);

		/*Generate an OpenGL 2D texture from the SDL_Surface*.*/
		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_2D, tex_id);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if ((strcmp(file,"stars.png") && strcmp(file,"stars.jpg") && strcmp(file,"stars_2.png")) == 0) { //TODO remove tmp code
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		} else {
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_ENUM_TYPE, img->pixels);
		SDL_FreeSurface(img);

		textures[tex_counter] = tex_id;

		//SDL_Log("DEBUG: Texture loaded: %s\n", file);
		return tex_counter;

	} else {
		SDL_Log("ERROR: Unable to load texture: %s\n IMG_ERROR: %s\n", filepath, IMG_GetError());
		return 0;
	}
}

static int texture_from_name(const char *file)
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
int texture_init(void)
{
	texture_load("error.png"); /* image to be shown for images which fails to load */

	TEX_GLOW_DOT = texture_load("dot.png"); //TODO move these definitions out of engine
	TEX_GLOW = texture_load("glow.png");
	//TEX_LIGHT = texture_load("light2.png");
	TEX_STARS = texture_load("stars.png");

	return 0;
}

int texture_destroy(void)
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
