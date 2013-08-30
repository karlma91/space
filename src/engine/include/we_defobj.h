/*
 * we_defobj.h
 *
 * Define OBJ_NAME and then include this header to define a new object type.
 */

/*** PER-OBJECT ***/
#ifdef OBJ_NAME

#define STRINGIFY_2(str) #str
#define STRINGIFY(str) STRINGIFY_2(str)

#define OBJ_TYPE OBJ_TYPE_2(OBJ_NAME)
#define OBJ_PARAM_TYPE OBJ_PARAM_2(OBJ_NAME)
#define OBJ_ID OBJ_ID_2(OBJ_NAME)

static void init(OBJ_TYPE *obj);
static void on_create(OBJ_TYPE *obj);
static void on_update(OBJ_TYPE *obj);
static void on_render(OBJ_TYPE *obj);
static void on_remove(OBJ_TYPE *obj);

static object_id this = {
	.OBJ_IDENTIFIER = OBJ_MAGIC_COOKIE,
	.ID = -1,
	.NAME = STRINGIFY(OBJ_NAME),
	.SIZE = sizeof(OBJ_TYPE),
	.P_SIZE = sizeof(OBJ_PARAM_TYPE),
	.call = {
		(void (*)(instance *)) init,
		(void (*)(instance *)) on_create,
		(void (*)(instance *)) on_update,
		(void (*)(instance *)) on_render,
		(void (*)(instance *)) on_remove
	}
};
object_id *OBJ_ID = &this;

#endif /* OBJ_TYPE_NAME */
