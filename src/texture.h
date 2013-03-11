#ifndef TEXTURE_H_
#define TEXTURE_H_

typedef struct texture_map {
	float ax;
	float ay;
	float bx;
	float by;
} texture_map;

extern int *textures;
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
	TEX_CLOUD;

#endif /* TEXTURE_H_ */
