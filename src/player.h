
struct player{
	cpBody *body;
	cpShape *shape;
	void (*init)(struct player *obj);
	void (*update)(struct player *obj);
	void (*render)(struct player *obj);
	void (*destroy)(struct player *obj);
	void *data;
};

extern struct player player;
