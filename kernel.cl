//#define WIDTH
//#define HEIGHT

typedef struct __attribute__((packed)) Movable {
    float x;
    float y;
    float vx;
    float vy;
    unsigned char hit;
} Movable;

kernel void update_pos(global Movable* objects) {
    int i = get_global_id(0);

    objects[i].x += objects[i].vx;
    objects[i].y += objects[i].vy;

    if (objects[i].x + objects[i].vx < 0 || objects[i].y + objects[i].vy < 0 || objects[i].x + objects[i].vx >= WIDTH || objects[i].y + objects[i].vy >= HEIGHT) {
        objects[i].hit = 1;
    }
}