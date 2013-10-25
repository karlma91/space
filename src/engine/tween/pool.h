#ifndef POOL_H_
#define POOL_H_


typedef struct pool pool;
struct pool {
	int element_size;
	LList available;
	//LList in_use;
};

pool * pool_create(int element_size);
void * pool_instance(pool *p);
void pool_release_rmcall(void *i, pool *p);
void pool_release(pool *p, void *i);
void pool_destroy(pool *p);

#endif /* POOL_H_ */
