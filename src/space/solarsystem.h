#ifndef SOLARSYSTEM_H_
#define SOLARSYSTEM_H_

//TODO MOVE INTO LEVEL FOLDER

#include "we_utils.h"
#include "we_graphics.h"
#include "we.h"

typedef struct solarsystem solarsystem;
typedef struct level_ship station;
typedef struct drawbl_sun drawbl_sun;

struct level_ship {
	solarsystem *sol;     // solarsystem station is in
	int index;            // index in solarsystem
	SPRITE_ID spr_id;     // sprite for drawing
	Color col;            // color for drawing
	cpVect pos;           // posistion used for drawing
	float radius;         // radius from center of sun
	float angle;          // current angle compared to sun
	float rotation_speed; // speed araound sun
	int data_place;       // is it in zip or in user_files?
	char filename[256];   // path to level
	char path[256];       // path to level
	char name[256];       // name of level
	char author[256];     // author of level
	button btn;           // btn to make station clickable
};

struct drawbl_sun {
	float size;
	SPRITE_ID spr_id;
	Color base, glow, add1, add2;
	float angvel, ang1, ang2;
	cpVect pos;
};

/* representing a space station system orbitting a sun */
struct solarsystem {
	int index;        // index in galaxy
	char name[256];   // name
	char folder[256];   // folder levels is in inder levels folder
	cpVect origo;     // place in space
	drawbl_sun sun;   // the sun
	LList stations;   // all stations in this solarsystem
};


solarsystem * solarsystem_get_from_folder(LList systems, char *folder);
station * solarsystem_get_station(solarsystem *ss, char *filename);
void solarsystem_register_touch(solarsystem *sol, STATE_ID id);
solarsystem *solarsystem_create(int solsys_index, char *name, char *folder);
void solarsystem_add_station(solarsystem * sol, SPRITE_ID spr_id, Color color, int dir_type, char * name, char * path, char * author, char *filename);
void solarsystem_remove_station(solarsystem *sy, station *s);
void solarsystem_update(solarsystem *sun);
void solarsystem_draw(solarsystem *sun);
void sun_render(int layer, drawbl_sun *sun);

LList solarsystem_load_solar(char *filepath);
void solarsystem_load_levels_from_folder(LList world);
void solarsystem_write_solar_file(LList world, char *filepath);


void solarsystem_destroy(solarsystem *sun);

#endif /* SOLARSYSTEM_H_ */
