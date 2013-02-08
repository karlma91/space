#ifndef LEVEL_H_
#define LEVEL_H_


typedef struct level_ship {
	float x;
	float y;
	int count;
	float radius;
	float rotation;
	float rotation_speed;
} level_ship;

typedef struct level_ {
	int station;
	int deck;
	int height;
	int level_left;
	int level_right;
} level;

extern int level_init();
extern level *level_load(int space_station, int deck);
extern void level_unload(level *);
extern void level_destroy();
extern void level_get_ships(level_ship **,int *);

#endif /* LEVEL_H_ */
