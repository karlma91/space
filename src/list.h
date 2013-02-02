extern void list_init();
extern void list_add(object *);
extern void list_iterate(void (*f)(object *));
extern void list_iterate_type(void (*f)(object *), int ID);
extern void list_destroy();
