//#ifndef SPACE_SPACE_HEADER
//#define SPACE_SPACE_HEADER
void SPACE_update(float dt);
void SPACE_draw(float dt);
void SPACE_init();
void SPACE_destroy();
extern state spaceState;

/* camera */
extern float cam_center_x;
extern float cam_center_y;
extern float cam_zoom;

extern cpSpace *space;


//#endif
