//#ifndef SPACE_MAIN_HEADER
//#define SPACE_MAIN_HEADER

#define SLEEP_TIME 1

/* State  struct */
typedef struct state_ state;
struct state_ {
	void (*update)(float dt);
	void (*render)(float dt);
	state *parentState;
};

/* super object struct */
typedef struct object_ object;
struct object_ {
	cpBody *body;
	cpShape *shape;
	void (*init)(object *obj);
	void (*update)(object *obj, float dt);
	void (*render)(object *obj, float dt);
	void (*destroy)(object *obj);
	void *data;
};

extern int WIDTH;
extern int HEIGHT;

extern Uint8 *keys;

extern char fps_buf[];
/* current state in use */
extern state *currentState;

//#endif

