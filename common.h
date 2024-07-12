#ifndef COMMON_H
#define COMMON_H

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define WIDTH 1280 //640
#define HEIGHT 960 //480

#define USE_WINDOW_SURFACE // This causes a dramatic speedup (~40+ more FPS) due to fewer constant writes between buffers (taking a more direct path). However, this has the possibility of causing undefined behavior due to surface-locking mechanisms.

//#define TARGET_FPS 60

typedef struct Color
{
    Uint8 b;
    Uint8 g;
    Uint8 r;
    Uint8 a;
} Color;

// This is a circular doubly linked-list. I'm so proud of myself for thinking of this, this feels kinda clever
// In the future we should try to vectorize some of this
typedef struct Laser {
    float x;
    float y;
    float vx;
    float vy;
    Color color;
    float length;
    struct Laser* prev_laser;
    struct Laser* next_laser;
} Laser;

typedef struct Enemy {
    float x;
    float y;
    float vx;
    float vy;
    Color color;
    float length;
    struct Enemy* prev_enemy;
    struct Enemy* next_enemy;
    unsigned char hit;
} Enemy;

/*union {
    Color color;
    int value;
} converter;*/

struct {
    unsigned char w_down : 1;
    unsigned char a_down : 1;
    unsigned char s_down : 1;
    unsigned char d_down : 1;
} key_table;

extern SDL_Window* window;

#ifndef USE_WINDOW_SURFACE
extern SDL_Renderer* renderer;
extern SDL_Texture* main_texture;
#else
extern SDL_Surface* win_surface;
#endif

extern Color* pixels;

extern int running;
extern int paused;

// Using arrays to encourage the compiler to vectorize code
extern float position[2];
extern unsigned int mouse_position[2];

extern float player_speed;
extern float enemy_speed;

extern int frames;
#ifdef TARGET_FPS
extern int curr_ticks;
#endif
extern int curr_ticks_fps;
extern int curr_ticks_enemy;

//Laser* head_laser = NULL;
extern Laser* current_laser;
extern Enemy* current_enemy;
extern int laser_count;
extern int enemy_count;

extern char display_str[48];

extern int fps;
extern int score;

extern int enemy_wait;

void Sleep(int millis);
void CreateLaser();
void KillCurrentLaser();
void CreateEnemy();
void KillCurrentEnemy();
void RenderChar(char chr, int x, int y, Color color);
void RenderString(const char* str, size_t length, int x, int y, Color color);

int AppInit();
int AppIterate();
int AppEvent(const SDL_Event* event);
void AppQuit();
#endif