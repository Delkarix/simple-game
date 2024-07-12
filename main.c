// Shiddely programmed™

// I've moved the various SDL_App* functions into different files for better ease-of-access but the linker became a huge pain in the ass so I have to do a manual C-file include, which is generally not good practice.
// I'll need to find an alternative or another workaround in the future for bigger projects.

// KEEP IN MIND THAT A RARE CRASH MAY OCCUR IF TWO NEIGHBORING LASERS/ENEMIES DIE AT THE SAME TIME: IT CAN CAUSE A DOUBLE-FREE

// FUTURE REFERENCE: SDL recommends dynamic linking over static linking so look into using DLL/SO files instead of static linking (i.e. -lSDL3)
// Although, some sources kinda give different implications which make things a little wonky

// Maybe make high score system?

// Make powerups

// Work on vectorization optimizations

// Maybe add keybind to toggle enemy movement/spawning?

/*

OPENCL TODO:
* Movement of all objects (player, enemies, lasers)
* Graphics manipulation and bulk pixel writing (drawing enemies, lasers, and player)
* Displaying text

Note that synchronous kernels may cause weird pixel-fighting issues
*/

#include "main.h"
#include "font.h"

// This is super dumb and bad but we need to manually include these because otherwise the linker loses its shit due to multiple definitions, some of which we might not be able to fix because they're out in the SDL headers.
// We need to find an alternative for the future because this will cripple memory use during compile time.
#ifdef USE_SDL3_SEMANTICS
#include "AppIterate.c"
#include "AppInit.c"
#include "AppEvent.c"
#include "AppQuit.c"
#endif

KeyTable key_table = {0};

SDL_Window* window = NULL;

#ifndef USE_WINDOW_SURFACE
SDL_Renderer* renderer = NULL;
SDL_Texture* main_texture = NULL;
#else
SDL_Surface* win_surface = NULL;
#endif

Color* pixels = NULL;

int running = 1;
int paused = 1;
int enemies_move = 1;
int enemies_spawn = 1;

// Using arrays to encourage the compiler to vectorize code
float position[2] = {WIDTH/2, HEIGHT/2};
unsigned int mouse_position[2] = {WIDTH/2, HEIGHT/2};

float player_speed = 2.0;
float enemy_speed = 1.0;

int frames = 0;
#ifdef TARGET_FPS
int curr_ticks;
#endif
int curr_ticks_fps;
int curr_ticks_enemy;

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

void CreateLaser() {
    if (!current_laser) {
        // If no lasers currently exist, create a new laser
        current_laser = (Laser*)SDL_malloc(sizeof(Laser));
        current_laser->prev_laser = current_laser;  // Set the previous laser to point back to itself
        current_laser->next_laser = current_laser;  // Set the next laser to point forward to itself
    }
    else {
        // If any lasers do currently exist, create a new laser and insert it in between the current laser and the next laser
        Laser* other_head = current_laser->next_laser; // Save the next laser
        current_laser->next_laser = (Laser*)SDL_malloc(sizeof(Laser)); // Create a new laser and insert it into the next laser
        current_laser->next_laser->prev_laser = current_laser; // Set the newly-created laser's previous laser to the current laser
        current_laser->next_laser->next_laser = other_head; // Set the newly-created laser's next laser to the original laser that came after the current laser.
        current_laser->next_laser->next_laser->prev_laser = current_laser->next_laser; // Set the original laser's previous laser to the new laser.
        current_laser = current_laser->next_laser;
        
    }

    current_laser->color.r = 0;
    current_laser->color.g = 0;
    current_laser->color.b = 255;
    current_laser->x = position[0];
    current_laser->y = position[1];

    // The 10* is an arbitrary speed. We might change that at some point but for now there isn't much of a point.
    current_laser->vx = 10*(mouse_position[0] - position[0])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    current_laser->vy = 10*(mouse_position[1] - position[1])/SDL_sqrt(SDL_pow(mouse_position[0] - position[0], 2) + SDL_pow(mouse_position[1] - position[1], 2));
    current_laser->length = 100; // Arbitrary length
    laser_count++;
}

void KillCurrentLaser() {
    if (laser_count == 1) {
        SDL_free(current_laser);
        current_laser = NULL;
    }
    else {
        // Stitch the elements back together
        current_laser->prev_laser->next_laser = current_laser->next_laser;
        current_laser->next_laser->prev_laser = current_laser->prev_laser;
        Laser* curr = current_laser;

        current_laser = current_laser->next_laser;
        SDL_free(curr);
    }

    laser_count--;
}

void CreateEnemy() {
    // See CreateLaser() for the concept.

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

    int try_x;
    int try_y;

    do {
        try_x = rand() % WIDTH;
        try_y = rand() % HEIGHT;
    } while (SDL_sqrt(SDL_pow(position[0] - try_x, 2) + SDL_pow(position[1] - try_y, 2)) < 100);

    current_enemy->x = try_x;
    current_enemy->y = try_y;
    current_enemy->length = 10; // Arbitrary again, may change this programmatically in the future
    current_enemy->hit = 0;
    enemy_count++;
}

void KillCurrentEnemy() {
    if (enemy_count == 1) {
        SDL_free(current_enemy);
        current_enemy = NULL;
    }
    else {
        // Stitch the elements back together
        current_enemy->prev_enemy->next_enemy = current_enemy->next_enemy;
        current_enemy->next_enemy->prev_enemy = current_enemy->prev_enemy;
        Enemy* curr = current_enemy;

        current_enemy = current_enemy->next_enemy;
        SDL_free(curr);
    }

    enemy_count--;
}

// Every char is 8x8
// NOTE: MUST ONLY USE UPPERCASE
void RenderChar(char chr, int x, int y, Color color) {
    for (int iy = 0; iy < 8; iy++) {
        for (int ix = 0; ix < 8; ix++) {
            // Iterates through every bit and if it is set, it is placed as a pixel.
            // Essentially, we're going backwards through the "traditional" bit order to emulate how it's structured in font.h
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

#ifndef USE_SDL3_SEMANTICS
int main(int argc, char** argv) {
    SDL_AppInit(NULL, argc, argv);
    SDL_Event e;

    while (running) {
        // This loop was particularly annoying to figure out, since apparently you HAVE to do SDL_PollEvent() != 0 instead of !SDL_PollEvent(), for whatever reason.
        while (SDL_PollEvent(&e) != 0) {
            SDL_AppEvent(NULL, &e);
        }

        SDL_AppIterate(NULL);
    }

    SDL_AppQuit(NULL);
    SDL_Quit();
}
#endif