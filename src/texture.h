#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "SDL_opengl.h"

#if TARGET_OS_IPHONE
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif


typedef struct texture_map {
	float map[8];
} texture_map;

extern GLuint *textures;
extern int texture_load(char *file);

extern int texture_init();

extern const texture_map *TEX_MAP_FULL;

extern int texture_bind();

int
	TEX_WHEEL,
	TEX_DOT,
	TEX_GLOW,
	TEX_GLOW,
	TEX_GLOWDOT,
	TEX_PLAYER,
	TEX_PARTICLE,
	TEX_BUTTON,
	TEX_BUTTON_DOWN,
	TEX_CLOUD_ULQ,
	TEX_CLOUD_LQ,
	TEX_CLOUD;

#endif /* TEXTURE_H_ */
