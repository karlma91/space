#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "SDL.h"
#include "SDL_image.h"

#include "waffle_utils.h"

#define MAX_IMAGE_BUFFER 1048576 /* ( 1 MiB) */

/**
 * texture values (GLOBAL)
 */
GLuint *textures;

static const texture_map TEX_MAP_FULL_ = {0,1, 1,1, 0,0, 1,0};
const texture_map *TEX_MAP_FULL = &TEX_MAP_FULL_;

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
	glEnable(GL_TEXTURE_2D);

	int have_texture = texture_from_name(file);
	if(have_texture >=0){
		//fprintf(stderr,"have texture: %s\n", file);
		return have_texture;
	}

	char filepath[64];
	sprintf(filepath,"textures/%s", file);

	ZZIP_FILE *zf;
	SDL_RWops *rw;

	if (!(zf = waffle_open(filepath))) {
		SDL_Log("Unable to find texture: %s", file);
	}

	char buffer[MAX_IMAGE_BUFFER];
	int file_size = zzip_read(zf, buffer, MAX_IMAGE_BUFFER);
	zzip_close(zf);

	rw = SDL_RWFromMem(&buffer[0], file_size);

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


		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		///glTexParameteri(GL_TEXTURE_2D,  GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if __ANDROID__
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_ENUM_TYPE, img->pixels);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_ENUM_TYPE, img->pixels);
#endif

		textures[tex_counter] = tex_id;

		SDL_FreeSurface(img);

		//fprintf(stderr,"Texture loaded: %s\n", file);

		return tex_counter;

	} else {
		fprintf(stderr,"Unable to load texture: %s\n IMG_ERROR: %s\n", filepath, IMG_GetError());
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
/* NEED TO BE CALLED BEFORE ANY texture_load() CALLS! */
extern int texture_init()
{
#if GLES2

#else
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
#endif
}

extern int texture_destroy()
{

}

int texture_bind(int tex_id) {
	static int last_tex_id = -1;
	if (tex_id != last_tex_id && tex_id >= 0) {
		last_tex_id = tex_id;
		glBindTexture(GL_TEXTURE_2D, textures[tex_id]);
		return 0;
	}
	return 1;
}
