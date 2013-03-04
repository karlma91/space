#ifndef TEXTURE_H_
#define TEXTURE_H_

typedef struct texture_map {
	float ax;
	float ay;
	float bx;
	float by;
} texture_map;

extern unsigned int *textures;
extern unsigned int texture_load(char *file);

#endif /* TEXTURE_H_ */
