#ifndef LIST_H_
#define LIST_H_

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

/* All collision types for objects */
enum OBJECT_ID {
	ID_PLAYER,
	ID_GROUND,
	ID_PLAYER_BULLET,
	ID_TANK_FACTORY,
	ID_TANK,

	ID_COUNT
};

extern void list_init();
extern void list_add(object *);
extern void list_iterate(void (*f)(object *));
extern void list_iterate_type(void (*f)(object *), int ID);
extern void list_destroy();


#endif /* LIST_H_ */
