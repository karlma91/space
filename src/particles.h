#define MAX_PARTICLES 50
#define MAX_EXPLOSIONS 500
#define MAX_EXPLOSION_TIME 1

void paricles_init();
void paricles_destroy();
void paricles_add_explosion(cpVect v, int numPar);
void paricles_draw(float dt);
