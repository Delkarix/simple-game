#ifndef MAIN_H
#define MAIN_H

// When compiling with USE_SDL3_SEMANTICS, select only common.c
// Otherwise, include all C files (*.c)
//#define USE_SDL3_SEMANTICS
#define USE_WINDOW_SURFACE // This causes a dramatic speedup (~40+ more FPS) due to fewer constant writes between buffers (taking a more direct path). However, this has the possibility of causing undefined behavior due to surface-locking mechanisms.
//#define TARGET_FPS 60
#define USE_OPENCL

#ifdef USE_SDL3_SEMANTICS
#define SDL_MAIN_USE_CALLBACKS
#endif

#ifdef USE_OPENCL
#include <CL/cl.h>
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define WIDTH 1280 //640
#define HEIGHT 960 //480

typedef struct Color
{
#ifdef USE_WINDOW_SURFACE
    Uint8 b;
    Uint8 g;
    Uint8 r;
    Uint8 a;
#else
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
#endif
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

typedef struct {
    unsigned char w_down : 1;
    unsigned char a_down : 1;
    unsigned char s_down : 1;
    unsigned char d_down : 1;
} KeyTable;

extern KeyTable key_table; // This is annoying

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
extern int enemies_move;
extern int enemies_spawn;

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

#ifndef USE_SDL3_SEMANTICS
int SDL_AppInit(void** appstate, int argc, char** argv);
int SDL_AppIterate(void* appstate);
int SDL_AppEvent(void* appstate, const SDL_Event* event);
void SDL_AppQuit(void* appstate);
#endif

#endif