
extern state state_space;

/* camera */
extern float cam_center_x;
extern float cam_center_y;
extern float cam_zoom;

/* level data */
extern int level_height;
extern int level_left;
extern int level_right;

/* chipmunk physics space */
extern cpSpace *space;

typedef struct object_ object;

struct obj_type {
	int ID;
	void (*init)(object *obj);
	void (*update)(object *obj);
	void (*render)(object *obj);
	void (*destroy)(object *obj);
};

struct object_ {
	struct obj_type *type;
	int instance_id;
	int *remove;
};

/* All collisoin types for objects */
enum OBJECT_ID {
	ID_PLAYER,
	ID_GROUND,
	ID_PLAYER_BULLET,
	ID_TANK_FACTORY,
	ID_TANK,

	ID_COUNT
};
