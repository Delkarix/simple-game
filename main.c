// Shiddely programmed™
// Prototype essentially
// Sorry future me for having really bad code, this is mostly just a proof-of-concept prototype
// In the future, we'll segregate the SDL_App* functions into different files

// KEEP IN MIND THAT A RARE CRASH MAY OCCUR IF TWO NEIGHBORING LASERS/ENEMIES DIE AT THE SAME TIME: IT CAN CAUSE A DOUBLE-FREE

// FUTURE REFERENCE: SDL recommends dynamic linking over static linking so look into using DLL/SO files instead of static linking (i.e. -lSDL3)
// Although, some sources kinda give different implications which make things a little wonky

// Maybe make high score system?

// Make powerups

// NOTE: Colors are kinda fuckedy atm, probably should fix em. Look into rearranging color struct values or change the rendering system altogether

#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "font.h"

#define WIDTH 1280 //640
#define HEIGHT 960 //480

#define USE_WINDOW_SURFACE // This causes a dramatic speedup (~40+ more FPS) due to fewer constant writes between buffers (taking a more direct path). However, this has the possibility of causing undefined behavior due to surface-locking mechanisms.

//#define TARGET_FPS 60

//#define ENEMY_COUNT 5
//#define LASER_PRECISION 100

typedef struct Color
{
    Uint8 b;
    Uint8 g;
    Uint8 r;
    Uint8 a;
} Color;

// This is a circular doubly linked-list. I'm so proud of myself for thinking of this, this feels kinda clever
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

union {
    Color color;
    int value;
} converter;

struct {
    unsigned char w_down : 1;
    unsigned char a_down : 1;
    unsigned char s_down : 1;
    unsigned char d_down : 1;
} key_table;

static SDL_Window* window = NULL;

#ifndef USE_WINDOW_SURFACE
static SDL_Renderer* renderer = NULL;
static SDL_Texture* main_texture = NULL;
#else
static SDL_Surface* win_surface = NULL;
#endif

static Color* pixels = NULL;
//static Color* blank_pixels = NULL;

//static SDL_Rect main_texture_rect = {0};
//static SDL_Rect window_rect = {0};

static int running = 1;
static int paused = 1;

// Using arrays to encourage the compiler to vectorize code
float position[2] = {WIDTH/2, HEIGHT/2};
unsigned int mouse_position[2] = {WIDTH/2, HEIGHT/2};

float player_speed = 2.0;
float enemy_speed = 1.0;
//float enemies_positions[ENEMY_COUNT][2] = {0}; // REMOVE

//unsigned long long ticks = 0;

//static int pixel_iterator = 0;
static int frames = 0;
#ifdef TARGET_FPS
int curr_ticks;
#endif
int curr_ticks_fps;
int curr_ticks_enemy;

//Laser* head_laser = NULL;
Laser* current_laser = NULL;
Enemy* current_enemy = NULL;
int laser_count = 0;
int enemy_count = 0;

char display_str[48];

int fps = 0;
int score = 0;

int enemy_wait = 500;

void Sleep(int millis) {
    long last_time = SDL_GetTicks();

    while (SDL_GetTicks() <= last_time + millis);
}

/*int fps_counter(void* data) {
    SDL_Log(""); // Reset line pos

    while (running) {
        Sleep(1000);
        fps = frames;
        enemy_wait -= 10; // every second, increase the enemy spawning rate.
        player_speed += 0.1; // Make player faster
        enemy_speed += 0.1; // Make enemies faster

        //SDL_Log("\x1b[1FFPS: %d     ", frames);
        frames = 0;
    }

    return 0;
}*/

void CreateLaser() {
    if (!current_laser) {
        current_laser = (Laser*)SDL_malloc(sizeof(Laser));
        current_laser->prev_laser = current_laser;
        current_laser->next_laser = current_laser;
    }
    else {
        Laser* other_head = current_laser->next_laser;
        current_laser->next_laser = (Laser*)SDL_malloc(sizeof(Laser));
        current_laser->next_laser->prev_laser = current_laser;
        current_laser->next_laser->next_laser = other_head;
        current_laser->next_laser->next_laser->prev_laser = current_laser->next_laser;
        current_laser = current_laser->next_laser;
        
    }

    current_laser->color.r = 0;
    current_laser->color.g = 0;
    current_laser->color.b = 255;
    current_laser->x = position[0];
    current_laser->y = position[1];
    current_laser->vx = 10*(mouse_position[0] - position[0])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    current_laser->vy = 10*(mouse_position[1] - position[1])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    //current_laser->x2 = position[0] + 20*(mouse_position[0] - position[0])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    //current_laser->y2 = position[1] + 20*(mouse_position[1] - position[1])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));;
    current_laser->length = 100; //10*SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));;
    laser_count++;
}

void KillCurrentLaser() {
    if (laser_count == 1) {
        //SDL_Log("test1");
        SDL_free(current_laser);
        current_laser = NULL;
    }
    else {
        //SDL_Log("test2");
        // Stitch the elements back together
        current_laser->prev_laser->next_laser = current_laser->next_laser;
        current_laser->next_laser->prev_laser = current_laser->prev_laser;
        Laser* curr = current_laser;

        current_laser = current_laser->next_laser;
        SDL_free(curr);
    }

    //SDL_Log("test3");
    laser_count--;
}

void CreateEnemy() {
    if (!current_enemy) {
        current_enemy = (Enemy*)SDL_malloc(sizeof(Enemy));
        current_enemy->prev_enemy = current_enemy;
        current_enemy->next_enemy = current_enemy;
    }
    else {
        Enemy* other_head = current_enemy->next_enemy;
        current_enemy->next_enemy = (Enemy*)SDL_malloc(sizeof(Enemy));
        current_enemy->next_enemy->prev_enemy = current_enemy;
        current_enemy->next_enemy->next_enemy = other_head;
        current_enemy->next_enemy->next_enemy->prev_enemy = current_enemy->next_enemy;
        current_enemy = current_enemy->next_enemy;
    }

    current_enemy->color.r = 255;
    current_enemy->color.g = 0;
    current_enemy->color.b = 0;

    int try_x;// = rand() % WIDTH;
    int try_y;// = rand() % HEIGHT;

    do {
        try_x = rand() % WIDTH;
        try_y = rand() % HEIGHT;
    } while (SDL_sqrt(SDL_pow(position[0] - try_x, 2) + SDL_pow(position[1] - try_y, 2)) < 100);

    current_enemy->x = try_x;
    current_enemy->y = try_y;
    //current_enemy->vx = 10*(mouse_position[0] - position[0])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    //current_enemy->vy = 10*(mouse_position[1] - position[1])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    //current_laser->x2 = position[0] + 20*(mouse_position[0] - position[0])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    //current_laser->y2 = position[1] + 20*(mouse_position[1] - position[1])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));;
    current_enemy->length = 10; //10*SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));;
    current_enemy->hit = 0;
    enemy_count++;
}

void KillCurrentEnemy() {
    if (enemy_count == 1) {
        //SDL_Log("test1");
        SDL_free(current_enemy);
        current_enemy = NULL;
    }
    else {
        //SDL_Log("test2");
        // Stitch the elements back together
        current_enemy->prev_enemy->next_enemy = current_enemy->next_enemy;
        current_enemy->next_enemy->prev_enemy = current_enemy->prev_enemy;
        Enemy* curr = current_enemy;

        current_enemy = current_enemy->next_enemy;
        SDL_free(curr);
    }

    //SDL_Log("test3");
    enemy_count--;
}

/*int EnemyGenerationLoop(void* data) {
    while (running) {
        Sleep(500);
        CreateEnemy();
    }

    return 0;
}*/

// Every char is 8x8
// NOTE: MUST ONLY USE UPPERCASE
void RenderChar(char chr, int x, int y, Color color) {
    for (int iy = 0; iy < 8; iy++) {
        for (int ix = 0; ix < 8; ix++) {
            // Iterates through every bit and if it is set, it is placed as a pixel.
            if (((FONT_CHAR_TABLE[chr] >> (63 - (iy*8 + ix))) & 1) == 1 && x+ix >= 0 && x+ix < WIDTH && y+iy >= 0 && y+iy < HEIGHT) {
                pixels[(y+iy)*WIDTH + x+ix] = color;
            }
        }
    }
}

// In the future, find a way to make the color change more dynamic (per char or even per pixel)
void RenderString(const char* str, size_t length, int x, int y, Color color) {
    int og_x = x;

    for (int i = 0; i < length; i++) {
        if (str[i] == '\n') {
            x = og_x;
            y += 9;
        }
        else {
            RenderChar(str[i], x, y, color);
            x += 9; // spacing by 9 instead of 8 to maintain at least 1 px between characters
        }
    }
}

int SDL_AppInit(void** appstate, int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO))
        return -1;

    window = SDL_CreateWindow("Window", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window)
        return -1;

#ifndef USE_WINDOW_SURFACE
    renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        return -1;

    SDL_RendererInfo sdlri = {0};
    SDL_GetRendererInfo(renderer, &sdlri);
    SDL_Log("Renderer name: %s, number of formats: %d\n", sdlri.name, sdlri.num_texture_formats);

    //pixels = win_surface->pixels; 
    pixels = SDL_malloc(sizeof(Color)*WIDTH*HEIGHT);
    //blank_pixels = SDL_malloc(sizeof(Color)*WIDTH*HEIGHT);

    if (!pixels)
        return -1;

    main_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    if (!main_texture)
        return -1;


#else
    win_surface = SDL_GetWindowSurface(window);
    
    if (!win_surface)
        return -1;

    SDL_LockSurface(win_surface);

    pixels = win_surface->pixels; // I doubt this will return 0
#endif
    // Randomize enemy positions
    for (int i = 0; i < 5; i++) {
        CreateEnemy();
    }

    /*main_texture_rect.w = WIDTH;
    main_texture_rect.h = HEIGHT;
    window_rect.w = WIDTH;
    window_rect.h = HEIGHT;*/

    //SDL_CreateThread(fps_counter, "fps_counter", NULL);
    //SDL_CreateThread(EnemyGenerationLoop, "enemy_gen_loop", NULL);

    SDL_ShowWindow(window);
    SDL_Log("starded");
    SDL_Log("%ld", sizeof(const unsigned long long));

#ifdef TARGET_FPS
    curr_ticks = SDL_GetTicks();
#endif
    curr_ticks_fps = SDL_GetTicks();
    curr_ticks_enemy = SDL_GetTicks();

    return 0;
}


int SDL_AppIterate(void* appstate) {
    if (!running) {
        return 1;
    }

#ifdef TARGET_FPS
    while (SDL_GetTicks() < curr_ticks + 1000/TARGET_FPS); // Force a specific FPS?
    curr_ticks = SDL_GetTicks();
#endif

    if (paused) {
        SDL_snprintf(display_str, sizeof(display_str), "FPS: %d\nSCORE: %d\nPAUSED", fps, score);
        RenderString(display_str, SDL_strlen(display_str), 0, 0, (Color){255, 255, 255, 255});

#ifndef USE_WINDOW_SURFACE
        SDL_UpdateTexture(main_texture, NULL, pixels, sizeof(Color)*WIDTH);
        SDL_RenderTexture(renderer, main_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
#else
        SDL_UpdateWindowSurface(window);
#endif
        //ticks++;

        return 0;
    }

    // Measure FPS
    int ticks_atm = SDL_GetTicks();
    if (ticks_atm >= curr_ticks_fps + 1000) {
        fps = frames;
        enemy_wait -= 10; // every second, increase the enemy spawning rate.
        player_speed += 0.1; // Make player faster
        enemy_speed += 0.1; // Make enemies faster

        //SDL_Log("\x1b[1FFPS: %d     ", frames);
        frames = 0;

        curr_ticks_fps = ticks_atm;
    }

    if (ticks_atm >= curr_ticks_enemy + enemy_wait) {
        CreateEnemy();
        curr_ticks_enemy = SDL_GetTicks();
    }    

    //memcpy(pixels, blank_pixels, sizeof(Color)*WIDTH*HEIGHT);
    /*SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = WIDTH;
    r.h = HEIGHT;
    SDL_FillSurfaceRect(win_surface, &r, 0);*/
    SDL_memset(pixels, 0, sizeof(Color)*WIDTH*HEIGHT);

    /*for (int i = 0; i < 500; i += 5) {
        pixels[i].r = 255;
        pixels[i+500].g = 255;
    }*/
    //pixel_iterator = (pixel_iterator + 1) % (WIDTH*HEIGHT);
    //converter.value = pixel_iterator;
    //pixels[pixel_iterator] = converter.color;

    /*for (int i = 0; i < WIDTH*HEIGHT; i++) {
        pixel_iterator++; //= (pixel_iterator + 1) % (WIDTH*HEIGHT);
        converter.value = pixel_iterator;
        pixels[i] = converter.color;
    }*/

    // Position update
    if (key_table.w_down && position[1] > 10) {
        position[1] -= player_speed;
    }
    if (key_table.a_down && position[0] > 10) {
        position[0] -= player_speed;
    }
    if (key_table.s_down && position[1] < HEIGHT - 11) {
        position[1] += player_speed;
    }
    if (key_table.d_down && position[0] < WIDTH - 11) {
        position[0] += player_speed;
    }

    // Update Lazers rahhhh
    //current_laser = head_laser;
    unsigned char dead_laser = 0;
    for (int i = 0; i < laser_count; i++) {
        current_laser->x += current_laser->vx; //0.1*(current_laser->x2 - current_laser->x1);
        current_laser->y += current_laser->vy; //0.1*(current_laser->y2 - current_laser->y1);
        //current_laser->x2 += 0.1*(current_laser->x2 - current_laser->x1);
        //current_laser->y2 += 0.1*(current_laser->y2 - current_laser->y1);

        // Test if any lasers hit enemies
        for (int j = 0; j < enemy_count; j++) {
            if (current_laser->x <= current_enemy->x + current_enemy->length && current_laser->x >= current_enemy->x - current_enemy->length && current_laser->y <= current_enemy->y + current_enemy->length && current_laser->y >= current_enemy->y - current_enemy->length) {
                dead_laser = 1;
                current_enemy->hit = 1;
                score++;
                SDL_Log("Killed Enemy");
            }

            current_enemy = current_enemy->next_enemy;
        }

        // x1 and y1 will always be closer to the player, which will always be on the inside of the drawing rectangle, so x2 and y2 are the only ones at risk of accidentally leaving the drawing area.
        //if (current_laser->x1 < 0 || current_laser->x1 >= WIDTH || current_laser->y1 < 0 || current_laser->y1 >= HEIGHT || current_laser->x2 < 0 || current_laser->x2 >= WIDTH || current_laser->y2 < 0 || current_laser->y2 >= HEIGHT) {
        if (dead_laser || current_laser->x + current_laser->vx < 0 || current_laser->x + current_laser->vx >= WIDTH || current_laser->y + current_laser->vy < 0 || current_laser->y + current_laser->vy >= HEIGHT) {
            KillCurrentLaser();
            SDL_Log("Laser Count: %d", laser_count);
        }
        //SDL_Log("test4");
        //else {
        if (current_laser && current_laser->next_laser) {
            current_laser = current_laser->next_laser;
        }
        
        //}
        //SDL_Log("test5");
    }

    for (int i = 0; i < enemy_count; i++) {
        if (current_enemy->hit) {
            KillCurrentEnemy();
        }

        if (current_enemy && current_enemy->next_enemy) {
            current_enemy = current_enemy->next_enemy;
        }
    }

    // Update enemies
    for (int i = 0; i < enemy_count; i++) {
        current_enemy->vx = enemy_speed*(position[0] - current_enemy->x)/SDL_sqrt(SDL_pow(position[0] - current_enemy->x, 2) + SDL_pow(position[1] - current_enemy->y, 2));
        current_enemy->vy = enemy_speed*(position[1] - current_enemy->y)/SDL_sqrt(SDL_pow(position[0] - current_enemy->x, 2) + SDL_pow(position[1] - current_enemy->y, 2));

        current_enemy->x += current_enemy->vx;
        current_enemy->y += current_enemy->vy; // No need to store vx and vy tbh

        // Determine if player should be dead
        if ((position[0] + 10 >= current_enemy->x - current_enemy->length && position[1] + 10 >= current_enemy->y - current_enemy->length) && (position[0] - 10 < current_enemy->x + current_enemy->length && position[1] - 10 < current_enemy->y + current_enemy->length)) {
            RenderString("GAME OVER", 10, WIDTH/2 - 40, HEIGHT/2 - 40, (Color){255, 255, 255, 255});
            paused = 1;
            //running = 0;
            SDL_Log("DEAD");
        }

        current_enemy = current_enemy->next_enemy;
    }

    // Draw player sprite thingy
    for (int iy = position[1]-10; iy < position[1]+10; iy++) {
        for (int ix = position[0]-10; ix < position[0]+10; ix++) {
            // Probably don't need this if-statement anymore since the rectangle can only be in-bounds (but who knows)
            if (iy >= 0 && iy < HEIGHT && ix >= 0 && ix < WIDTH) {
                pixels[iy*WIDTH + ix].g = 255;
            }
        }
    }
    //pixels[position[1]*WIDTH + position[0]].r = 255;

    // Draw sprite at mouse pos
    for (int iy = mouse_position[1]-5; iy < mouse_position[1]+5; iy++) {
        for (int ix = mouse_position[0]-5; ix < mouse_position[0]+5; ix++) {
            if (iy >= 0 && iy < HEIGHT && ix >= 0 && ix < WIDTH) {
                pixels[iy*WIDTH + ix].b = 255;
            }
        }
    }

    // Draw Enemies
    for (int i = 0; i < enemy_count; i++) {
        for (int iy = current_enemy->y - current_enemy->length; iy < current_enemy->y + current_enemy->length; iy++) {
            for (int ix = current_enemy->x - current_enemy->length; ix < current_enemy->x + current_enemy->length; ix++) {
                if (iy >= 0 && iy < HEIGHT && ix >= 0 && ix < WIDTH) {
                    pixels[iy*WIDTH + ix] = current_enemy->color;
                }
            }
        }

        current_enemy = current_enemy->next_enemy;
    }

    // Draw Laz3r5
    //current_laser = head_laser;
    for (int i = 0; i < laser_count; i++) {
        float ix = current_laser->x;
        float iy = current_laser->y;

        for (int j = 0; j < SDL_ceil(current_laser->length); j++) {
            // This (int)iy (int)ix thing is so weird and was a bitch-and-a-half to figure out
            pixels[(int)iy*WIDTH + (int)ix] = current_laser->color;

            ix += current_laser->vx/current_laser->length;
            iy += current_laser->vy/current_laser->length;
            //SDL_Log("%f %f %f %d %f", iy*WIDTH + ix, ix, iy, laser_count, lasers[i].x2 - lasers[i].x1);
            //getc(stdin);
        }

        current_laser = current_laser->next_laser;
    }

    //RenderChar('A', 0, 0, (Color){255, 255, 255, 255});
    
    if (!paused) {
        SDL_snprintf(display_str, sizeof(display_str), "FPS: %d\nSCORE: %d", fps, score);
        RenderString(display_str, SDL_strlen(display_str), 0, 0, (Color){255, 255, 255, 255});
    }
    
    // Render the pixel buffer to the screen
#ifndef USE_WINDOW_SURFACE
    SDL_UpdateTexture(main_texture, NULL, pixels, sizeof(Color)*WIDTH); // Updates the main drawing texture
    SDL_RenderTexture(renderer, main_texture, NULL, NULL); // Writes the main window texture to the renderer
    SDL_RenderPresent(renderer); // Makes the renderer draw to the window
#else
    SDL_UpdateWindowSurface(window);
#endif
    
    //memcpy(win_surface->pixels, pixels, sizeof(Color)*WIDTH*HEIGHT);

    frames++;
    //ticks++;
    return 0;
}

// In the future, we'll move this to a different file
int SDL_AppEvent(void* appstate, const SDL_Event* event) {
    if (!running) {
        return 1;
    }
    
    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
            switch (event->key.keysym.sym) {
                case SDLK_q:
                    //SDL_AppQuit(appstate); // This is probably bad lol
                    running = 0;
                    //exit(0); // I'd rather not do this, if only there was a way to do it from within SDL itself.
                    break;
                case SDLK_w:
                    key_table.w_down = 1;
                    break;
                case SDLK_a:
                    key_table.a_down = 1;
                    break;
                case SDLK_s:
                    key_table.s_down = 1;
                    break;
                case SDLK_d:
                    key_table.d_down = 1;
                    break;
                case SDLK_ESCAPE:
                    paused = !paused;
                    break;
            }
            break;

        case SDL_EVENT_KEY_UP:
            switch (event->key.keysym.sym) {
                case SDLK_q:
                    running = 0;
                    //SDL_AppQuit(appstate); // This is probably bad lol
                    //exit(0); // I'd rather not do this, if only there was a way to do it from within SDL itself.
                    break;
                case SDLK_w:
                    key_table.w_down = 0;
                    break;
                case SDLK_a:
                    key_table.a_down = 0;
                    break;
                case SDLK_s:
                    key_table.s_down = 0;
                    break;
                case SDLK_d:
                    key_table.d_down = 0;
                    break;
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            mouse_position[0] = event->motion.x;
            mouse_position[1] = event->motion.y;
            break;
        
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            SDL_Log("laser");
            CreateLaser();
            SDL_Log("Laser Count: %d", laser_count);
            SDL_Log("Laser velocity: %f", current_laser->vx);
            break;
    }

    return 0;
}

// This will be called whenever AppIterate() returns 1
void SDL_AppQuit(void* appstate) {
    SDL_Log("quidding\n");

    running = 0;

#ifndef USE_WINDOW_SURFACE
    if (main_texture)
        SDL_DestroyTexture(main_texture);
        main_texture = NULL;

    if (pixels)
        SDL_free(pixels);
        pixels = NULL;

    if (renderer)
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
#else
    SDL_UnlockSurface(win_surface);
#endif
    

    if (window)
        SDL_DestroyWindow(window);
        window = NULL;

    SDL_Log("freed shit");
}