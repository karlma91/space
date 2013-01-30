//#ifndef SPACE_MAIN_HEADER
//#define SPACE_MAIN_HEADER

#define SLEEP_TIME 1

/* State  struct */
typedef struct state_ state;
struct state_ {
	void (*init)();
	void (*update)();
	void (*render)();
	void (*destroy)();
	state *parentState;
};

extern int WIDTH;
extern int HEIGHT;
/* The delta time */
extern float dt;

extern Uint8 *keys;

extern char fps_buf[];
/* current state in use */
extern state *currentState;

extern int main_stop();

//#endif

