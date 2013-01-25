#define MAX_PARTICLES 50
#define MIN_PARTICLES 5
#define MAX_EXPLOSIONS 500
#define MAX_EXPLOSION_TIME 0.5f

void particles_init();
void particles_destroy();
void particles_add_explosion(cpVect v, int numPar);
void particles_draw();
void particles_update();
