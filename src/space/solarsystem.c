#include "solarsystem.h"
#include "game.h"
#include "states/levelscreen.h"


static char DEF_STRING[10] = "SOLARNULL";

//TODO MOVE INTO LEVEL FOLDER

static void station_free(station *s)
{
	free(s);
}

void solarsystem_create_drawbl(drawbl_sun * sun, float star_size, SPRITE_ID star_spr, Color star_base, Color star_glow, Color star_add1, Color star_add2)
{
	sun->size = star_size;
	sun->base = star_base;
	sun->glow = star_glow;
	sun->add1 = star_add1;
	sun->add2 = star_add2;
	sun->spr_id = star_spr;
	sun->angvel = 0.03;
}

solarsystem *solarsystem_create(int solsys_index, char *name, char *folder)
{
	solarsystem *solsys = (solarsystem *)calloc(1, sizeof *solsys);
	solsys->index = solsys_index;

	strcpy(solsys->name, name);
	strcpy(solsys->folder, folder);

	int i = solsys_index + 2;
	cpFloat radius = powf(i, 0.8) * 4800;
	cpFloat angle = WE_2PI * powf(i, 0.8) / 3.768226535;//  + we_randf / 50);
	i = 2;
	cpFloat off_radius = powf(i, 0.8) * 4800;
	cpFloat off_angle = WE_2PI * powf(i, 0.8) / 3.768226535;//  + we_randf / 50);

	cpVect jitter = cpvmult(cpv(we_randf - 0.5, we_randf - 0.5), 100);
	cpVect offset = WE_P2C(off_radius, off_angle);
	solsys->origo = cpvsub(cpvadd(WE_P2C(radius,angle), jitter), offset);

	solsys->sun.pos = solsys->origo;
	solsys->sun.angvel = 0.03;

	SPRITE_ID spr_sun = sprite_link("sun01");
	float rnd = rand() & 0x1f;
	Color base = {0x70-rnd,0x30,0x30+rnd,0xff};
	Color glow = {0xff-rnd,0xa0,0x70+rnd,0x80};
	Color add1 = {0x90-rnd,0x80,0x40+rnd,0x00};
	Color add2 = {0xb0-rnd,0x70,0x40+rnd,0x00};
	int size = (500 + we_randf*(solsys_index*1500/2 + (solsys_index>1?1000:0)) + 300*solsys_index/2);
	solarsystem_create_drawbl(&(solsys->sun),size, spr_sun,base,glow,add1,add2);
	solsys->stations = llist_create();
	return solsys;
}

static void button_callback(void *data)
{
	levelscreen_change_to(data);
}

void solarsystem_add_station(solarsystem * sol, SPRITE_ID spr_id, Color color, int dir_type, char * name, char * path, char * author, char *filename)
{
	station *s = calloc(1, sizeof(station));
	s->sol = sol;
	strcpy(s->path, path);
	strcpy(s->name, name);
	strcpy(s->author, author);
	strcpy(s->filename, filename);
	int i = llist_size(sol->stations);
	float size = 300 + we_randf * 100;
	cpFloat radius = sol->sun.size + (i) * 400 ;
	cpFloat angle = WE_2PI * we_randf;
	s->radius = radius;
	s->angle = angle;
	s->rotation_speed = 1000/radius;
	s->pos = cpvadd(WE_P2C(radius,angle), sol->origo);
	s->col = color;
	Color col_back = {255,200,180,255};//{255,180,140,255};
	s->btn = button_create(SPRITE_STATION001, 0, s->name, s->pos.x, s->pos.y, size, size);
	button_set_click_callback(s->btn, button_callback, s);
	button_set_txt_antirot(s->btn, 1);
	button_set_backcolor(s->btn, col_back);
	button_set_animated(s->btn, 1, (i ? 18 : 15));
	button_set_enlargement(s->btn, 1.5);
	button_set_hotkeys(s->btn, digit2scancode[(i+1) % 10], 0);
	state_register_touchable(state_stations, s->btn);
	sprite *spr = button_get_sprite(s->btn);
	spr->antirot = 1;
	llist_add(sol->stations, s);
}

void solarsystem_remove_station(solarsystem *sy, station *s)
{
	if(llist_remove(sy->stations, s)) {
		if(state_remove_touchable(state_stations, s->btn)){
			SDL_Log("Could not remove station touchable");
		}
	}else{
		SDL_Log("Could not remove station");
	}
}

void solarsystem_register_touch(solarsystem *sol, STATE_ID id)
{
	/*llist_begin_loop(sol->stations);
	while (llist_hasnext(sol->stations)) {
		state_register_touchable(id,((station *)llist_next(sol->stations))->btn);
	}
	llist_end_loop(sol->stations);*/
}

void solarsystem_update(solarsystem *sol)
{
	llist_begin_loop(sol->stations);
	while (llist_hasnext(sol->stations)) {
		station *s = ((station *)llist_next(sol->stations));
		s->angle += 0.4/s->radius;
		s->pos = cpvadd(WE_P2C(s->radius,s->angle), sol->origo);
		touch_place(s->btn,s->pos.x, s->pos.y);
	}
	llist_end_loop(sol->stations);

}

void solarsystem_draw(solarsystem *sol)
{
	sun_render(RLAY_GAME_BACK, &sol->sun);
	llist_begin_loop(sol->stations);
	draw_color4b(50,50,50,50);
	while (llist_hasnext(sol->stations)) {
		cpVect p = ((station *)llist_next(sol->stations))->pos;
		float r = cpvlength(cpvsub(sol->origo,p));
		float w = 1 / current_view->zoom;
		draw_donut(RLAY_GAME_BACK, sol->origo, r - w, r + w);
	}
	llist_end_loop(sol->stations);
}


void sun_render(int layer, drawbl_sun *sun)
{
	cpVect size = cpv(sun->size,sun->size);
	draw_color(sun->base);
	sprite_render_index_by_id(layer, sun->spr_id, 0, sun->pos, size, 0);
	draw_color(sun->glow);
	sprite_render_index_by_id(layer, SPRITE_GLOW, 0, sun->pos, cpvmult(size,2), 0);
	draw_color(sun->add1);
	sprite_render_index_by_id(layer, sun->spr_id, 1, sun->pos, size, sun->ang1);
	draw_color(sun->add2);
	sprite_render_index_by_id(layer, sun->spr_id, 2, sun->pos, size, sun->ang2);

	float spd = WE_2PI * dt * sun->angvel;
	sun->ang1 += spd;
	sun->ang2 -= spd;

}

solarsystem * solarsystem_get_from_folder(LList systems, char *folder)
{
	solarsystem *solar = NULL;
	llist_begin_loop(systems);
	while (llist_hasnext(systems)) {
		solarsystem *s = llist_next(systems);
		if (strcmp(s->folder, folder) == 0) {
			solar = s;
			break;
		}
	}
	llist_end_loop(systems);
	return solar;
}

station * solarsystem_get_station(solarsystem *ss, char *path)
{
	station *found_st = NULL;
	llist_begin_loop(ss->stations);
	while (llist_hasnext(ss->stations)) {
		station *st = llist_next(ss->stations);
		SDL_Log("%s == %s", st->path, path);
		if (strcmp(st->path, path) == 0) {
			found_st = st;
			break;
		}
	}
	llist_end_loop(ss->stations);
	return found_st;
}

static char * color_to_string(Color c, char *t){
	sprintf(t,"%02X%02X%02X%02X", c.r, c.b, c.g, c.a);
	return t;
}

static LList load_solarsystem_file(cJSON *root)
{
	SDL_Log("LOADING SOLARSYSTEMS");
	LList world = llist_create();
	llist_set_remove_callback(world, (ll_rm_callback)solarsystem_destroy);


	cJSON *systems = cJSON_GetObjectItem(root,"systems");
	if(systems) {
		int i,j;
		char name[256];
		char folder[256];
		char path[256];
		char filename[256];
		char author[256];
		SPRITE_ID spr;
		Color col;
		int size;
		for (i=0; i < cJSON_GetArraySize(systems); i++) {
			cJSON * sun = cJSON_GetArrayItem(systems,i);
			jparse_parse(sun, "name", "char", &name, DEF_STRING);
			jparse_parse(sun, "folder", "char", &folder, DEF_STRING);
			SDL_Log("CREATING SUN)");
			solarsystem *sy = solarsystem_create(i, name, folder);

			cJSON * levels = cJSON_GetObjectItem(sun,"levels");
			for (j= 0; j < cJSON_GetArraySize(levels); j++) {
				cJSON * lvl = cJSON_GetArrayItem(levels,j);
				jparse_parse(lvl, "author", "char", author, DEF_STRING);
				jparse_parse(lvl, "name", "char", name, DEF_STRING);
				jparse_parse(lvl, "filename", "char", filename, DEF_STRING);
				jparse_parse(lvl, "sprite", "sprite", &(spr), SPRITE_STATION001);
				jparse_parse(lvl, "size", "int ", &size, 0);
				jparse_parse(lvl, "color", "Color", &col, 0);
				sprintf(path, "levels/%s/%s", folder, filename);
				FILE* f =  waffle_fopen(WAFFLE_DOCUMENTS, path, "r");
				if(f == NULL){
					SDL_Log("%s does not exist", path);
				}else{
					fclose(f);
					SDL_Log("CREATING station %s", path);
					solarsystem_add_station(sy, spr,col, WAFFLE_DOCUMENTS, name , path, author, filename);
				}
			}
			llist_add(world,sy);
		}
		/*if(llist_size(sy->stations)){
				llist_add(world,sy);
			}else{
				solarsystem_destroy(sy);
			}*/
	} else {
		SDL_Log("Could not find field systems");
		SDL_Log("%s", cJSON_GetErrorPtr());
	}
	return world;
}
void solarsystem_load_levels_from_folder(LList world)
{
	we_diriter *wed = waffle_get_diriter(WAFFLE_DOCUMENTS, "levels");
	while(waffle_dirnext(wed)){
		if(waffle_isdir(wed)) {
			solarsystem *sy = solarsystem_get_from_folder(world, wed->dir->d_name);
			if(sy == NULL){
				SDL_Log("Creating new solarsystem %s,  %s", wed->dir->d_name, wed->dir->d_name );
				sy = solarsystem_create(llist_size(world), wed->dir->d_name, wed->dir->d_name);
				llist_add(world, sy);
			}else{
				SDL_Log("Found solarsystem for folder %s", wed->cur_path);
			}
			we_diriter *wed2 = waffle_get_diriter(WAFFLE_DOCUMENTS, wed->cur_path);
			while(waffle_dirnext(wed2)) {
				if(waffle_isfile(wed2)) {
					SDL_Log("LOOKING FOR %s",  wed2->dir->d_name);
					station *st = solarsystem_get_station(sy, wed2->cur_path);
					if(st == NULL){
						SDL_Log("LEVELS ADDING: %s", wed2->cur_path);
						char temp[64];
						sscanf(wed2->dir->d_name, "%s.json", temp);
						temp[strlen(temp) - 5] = 0;
						solarsystem_add_station(sy, SPRITE_COIN, COL_BLACK, wed2->dir_type, temp , wed2->cur_path, "author", wed2->dir->d_name);
					}else{
						SDL_Log("Found station %s in folder %s",wed2->dir->d_name, wed2->path);
					}
				}
			}
		}
	}
	waffle_free_diriter(wed);
}


LList solarsystem_load_solar(char *filepath)
{
	cJSON *root = jparse_open(WAFFLE_DOCUMENTS, filepath);
	if(root == NULL){
		SDL_Log("[SOLARSYSTEM] could not parse file: %s", filepath);
		SDL_Log("Error before: [%s]\n",cJSON_GetErrorPtr());
        root = cJSON_CreateObject();
    }
	LList userlist = load_solarsystem_file(root);
	solarsystem_load_levels_from_folder(userlist);
	return userlist;
}

void solarsystem_write_solar_file(LList world, char *filepath)
{
	FILE *file = waffle_fopen(WAFFLE_DOCUMENTS, filepath,"w");
	if (file == NULL) {
		SDL_Log( "Could not open levels/userlevels.json");
		return;
	}
	cJSON *root;//,*fmt;
	root = cJSON_CreateObject();
	cJSON * systems = cJSON_CreateArray();
	llist_begin_loop(world);
	while (llist_hasnext(world)) {
		solarsystem *sy = llist_next(world);

		cJSON *system = cJSON_CreateObject();
		cJSON_AddItemToObject(system, "name", cJSON_CreateString(sy->name));
		cJSON_AddItemToObject(system, "folder", cJSON_CreateString(sy->folder));

		cJSON * levels = cJSON_CreateArray();
		llist_begin_loop(sy->stations);
		while (llist_hasnext(sy->stations)) {
			station *st = llist_next(sy->stations);

			cJSON *station = cJSON_CreateObject();
			cJSON_AddItemToObject(station, "author", cJSON_CreateString(st->author));
			cJSON_AddItemToObject(station, "name", cJSON_CreateString(st->name));
			cJSON_AddItemToObject(station, "filename", cJSON_CreateString(st->filename));
			cJSON_AddItemToObject(station, "sprite", cJSON_CreateString(sprite_get_name(st->spr_id)));
			cJSON_AddNumberToObject(station, "size", st->radius);
			char c[10];
			cJSON_AddItemToObject(station, "color", cJSON_CreateString(color_to_string(st->col, c)));

			cJSON_AddItemToArray(levels, station);
		}
		llist_end_loop(sy->stations);

		cJSON_AddItemToObject(system, "levels", levels);
		cJSON_AddItemToArray(systems, system);
		cJSON_AddItemToObject(root, "systems", systems);
	}
	llist_end_loop(world);
	char * rendered = cJSON_Print(root);
	fprintf(file,"%s",rendered);
	fclose(file);
	cJSON_Delete(root);

}


void solarsystem_destroy(solarsystem *sy)
{
	llist_set_remove_callback(sy->stations, (ll_rm_callback)station_free);
	llist_destroy(sy->stations);
	free(sy);
}
