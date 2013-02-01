#define MAX_PARTICLES 50
#define MIN_PARTICLES 5
#define MAX_EXPLOSIONS 200
#define MAX_EXPLOSION_TIME 0.4f

void particles_init();
void particles_destroy();
void particles_add_explosion(cpVect v, float time, int speed, int numPar, int color);
void particles_draw();
void particles_update();
