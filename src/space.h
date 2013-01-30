
extern state state_space;

/* camera */
extern float cam_center_x;
extern float cam_center_y;
extern float cam_zoom;

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
	void *data;
};
