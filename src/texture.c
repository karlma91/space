#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_image.h"

/**
 * texture values (GLOBAL)
 */
int *textures;

static const texture_map TEX_MAP_FULL_ = {0,0,1,1};
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
static GLenum GL_ENUM_TYPE;

int texture_load(char *file)
{

	int have_texture = texture_from_name(file);
	if(have_texture >=0){
		fprintf(stderr,"have texture: %s\n", file);
		return have_texture;
	}

	char filepath[100];
	sprintf(filepath,"textures/%s",file);

	SDL_Surface* Surf_Temp = NULL;
	SDL_Surface* Surf_Return = NULL;

	if((Surf_Temp = IMG_Load(filepath)) == NULL) {
		fprintf(stderr,"Unable to load texture: %s\n", filepath);
		return -1;
	}

	tex_counter++;
	names = realloc(names,(sizeof(char[tex_counter+1][51])));
	strcpy(names[tex_counter],file);
	textures = realloc(textures,sizeof(int[(tex_counter + 1)]));


	Surf_Return = SDL_DisplayFormatAlpha(Surf_Temp);

	unsigned int Tex = 0;

	/*Generate an OpenGL 2D texture from the SDL_Surface*.*/
	glGenTextures(1, &Tex);
	glBindTexture(GL_TEXTURE_2D, Tex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
/*
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surf_Return->w, Surf_Return->h, 0, GL_BGRA,
			GL_UNSIGNED_BYTE, Surf_Return->pixels);
*/
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surf_Return->w, Surf_Return->h, 0, GL_BGRA,
			GL_ENUM_TYPE, Surf_Return->pixels);

	textures[tex_counter] = Tex;

	SDL_FreeSurface(Surf_Temp);
	SDL_FreeSurface(Surf_Return);

	fprintf(stderr,"loaded texture: %s\n", file);
	return tex_counter;
}

static int texture_from_name(char *file)
{
	//fprintf(stderr,"line: %d\n", __LINE__);
	int i;
	for(i=0;i<=tex_counter; i++){
		if(strcmp(names[i],file) == 0){
			return i;
		}
	}
	//fprintf(stderr,"line: %d\n", __LINE__);
	return -1;
}

/* NEED TO BE CALLED BEFORE ANY texture_load() CALLS! */
extern int texture_init(GLenum type)
{
	GL_ENUM_TYPE = type;

	TEX_CLOUD = texture_load("cloud.png");
	TEX_CLOUD_ULQ = texture_load("cloud_ultralow.png");
	TEX_CLOUD_LQ = texture_load("cloud_low.png");
	TEX_WHEEL   = texture_load("wheel.png");
	TEX_DOT     = texture_load("dot.png");
	TEX_GLOW    = texture_load("glow.png");
	TEX_GLOWDOT = texture_load("glowdot.png");
	TEX_PLAYER = texture_load("player.png");

	return 0;
}

int texture_bind(int tex_id) {
	if (tex_id >= 0) {
		glBindTexture(GL_TEXTURE_2D, textures[tex_id]);
		return 0;
	}
	return 1;
}
