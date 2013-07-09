

typedef struct _camera {

    int mode;
    float left;
    float right;
    float width;
    float height;
    float x;
    float y;
    float zoom;
    float rotation;

} camera;


void camera_move(camera * cam, float x, float y);
void camera_movex(camera * cam, float x);
void camera_movetox(camera * cam, float x);
void camera_translate(camera *cam);
void camera_rotate(camera *cam, float a);
