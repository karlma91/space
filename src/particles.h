#define MAX_PARTICLES 50
#define MIN_PARTICLES 5
#define MAX_EXPLOSIONS 500
#define MAX_EXPLOSION_TIME 0.5f

void paricles_init();
void paricles_destroy();
void paricles_add_explosion(cpVect v, int numPar);
void paricles_draw(float dt);
void particles_update(float dt);
