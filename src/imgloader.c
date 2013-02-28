
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_image.h"

/**
 * texture values (GLOBAL)
 */
unsigned int *textures;

/**
 * Texture names
 */
static char (*names)[20];

/**
 * number of textures
 */
static int tex_counter = -1;

static int texture_from_name(char *file);

unsigned int loadePNGTexture(char *file)
{
	int have_texture = texture_from_name(file);
	if(have_texture >=0){
		return have_texture;
	}

	SDL_Surface* Surf_Temp = NULL;
	SDL_Surface* Surf_Return = NULL;

	if((Surf_Temp = IMG_Load(file)) == NULL) {
		printf("Unable to load texture: %s\n", file);
		return -1;
	}

	tex_counter++;
	names = realloc(names,(sizeof(char[tex_counter+1][20])));
	textures = realloc(textures,sizeof(int[(tex_counter + 1)]));


	Surf_Return = SDL_DisplayFormatAlpha(Surf_Temp);

	unsigned int Tex = 0;

	/*Generate an OpenGL 2D texture from the SDL_Surface*.*/
	glGenTextures(1, &Tex);
	glBindTexture(GL_TEXTURE_2D, Tex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surf_Return->w, Surf_Return->h, 0, GL_BGRA,
			GL_UNSIGNED_BYTE, Surf_Return->pixels);

	textures[tex_counter] = Tex;

	SDL_FreeSurface(Surf_Temp);
	SDL_FreeSurface(Surf_Return);

	return tex_counter;
}

static int texture_from_name(char *file)
{
	int i;
	for(i=0;i<tex_counter; i++){
		if(strcmp(names[i],file) == 0){
			return i;
		}
	}
	return -1;
}
