//#ifndef SPACE_MAIN_HEADER
//#define SPACE_MAIN_HEADER

#define SLEEP_TIME 1

/* State  struct */
struct state {
    void (*update)(float dt);
    void (*render)(float dt);
    struct state *parentState;
};

extern int WIDTH;
extern int HEIGHT;

extern Uint8 *keys;

extern char fps_buf[];
/* current state in use */
extern struct state *currentState;

//#endif
