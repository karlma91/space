
struct player{
	struct obj_type *type;
	int instance_id;
	int *remove;
	cpBody *body;
	cpShape *shape;
	float max_hp;
	float hp;
};

extern struct obj_type type_player;
struct player player;
object *player_init();
