#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "draw.h"

extern GLuint *textures;
extern int texture_load(char *file);

extern int texture_init();

extern const float TEX_MAP_FULL[8];

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
