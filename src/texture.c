#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_image.h"

/**
 * texture values (GLOBAL)
 */
GLuint *textures;

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

	SDL_Surface* img = NULL;
	SDL_Surface* img_rgba = NULL;

	if((img = IMG_Load(filepath)) == NULL) {
		fprintf(stderr,"Unable to load texture: %s\n", filepath);
		return -1;
	}

	++tex_counter;
	names = realloc(names,(sizeof(char[tex_counter+1][51])));
	strcpy(names[tex_counter],file);
	textures = realloc(textures,sizeof(int[(tex_counter + 1)]));

	{
		/* OpenGL pixel format for destination surface. */
		int bpp;
		Uint32 Rmask, Gmask, Bmask, Amask;
		SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp, &Rmask,
				&Gmask, &Bmask, &Amask);

		/* Create surface that will hold pixels passed into OpenGL. */
		SDL_Surface *img_rgba8888 = SDL_CreateRGBSurface(0, img->w, img->h, bpp,
				Rmask, Gmask, Bmask, Amask);

		/*
		 * Disable blending for source surface. If this is not done, all
		 * destination surface pixels end up with crazy alpha values.
		 */
		SDL_SetSurfaceAlphaMod(img, 0xFF);
		SDL_SetSurfaceBlendMode(img, SDL_BLENDMODE_NONE);

		/* Blit to this surface, effectively converting the format. */
		SDL_BlitSurface(img, NULL, img_rgba8888, NULL);
	} //ref -> http://sdl.5483.n7.nabble.com/SDL-2-0-Surface-to-OpenGL-Texture-td22971.html



	img_rgba = SDL_ConvertSurfaceFormat(img,SDL_PIXELFORMAT_RGBA8888,0);

	unsigned int Tex = 0;

	/*Generate an OpenGL 2D texture from the SDL_Surface*.*/
	glGenTextures(1, &Tex);
	fprintf(stderr,"LINE: %d  GL_ERROR: %d\n",__LINE__,glGetError()); //TODO REMOVE

	glBindTexture(GL_TEXTURE_2D, Tex);
	fprintf(stderr,"LINE: %d  GL_ERROR: %d\n",__LINE__,glGetError()); //TODO REMOVE

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,  GL_GENERATE_MIPMAP, GL_TRUE);
/*
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_rgba->w, Surf_Return->h, 0, GL_BGRA,
			GL_UNSIGNED_BYTE, img_rgba->pixels);
*/

	fprintf(stderr,"LINE: %d  GL_ERROR: %d\n",__LINE__,glGetError()); //TODO REMOVE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_rgba->w, img_rgba->h, 0,
			GL_RGBA, GL_ENUM_TYPE, img_rgba->pixels);
	fprintf(stderr,"LINE: %d  GL_ERROR: %d\n",__LINE__,glGetError()); //TODO REMOVE

	textures[tex_counter] = Tex;

	SDL_FreeSurface(img);
	SDL_FreeSurface(img_rgba);

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
