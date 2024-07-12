#include "main.h"

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

        if (enemies_spawn) {
            enemy_wait -= 10; // every second, increase the enemy spawning rate.
        }
        
        player_speed += 0.1; // Make player faster

        if (enemies_move) {
            enemy_speed += 0.1; // Make enemies faster
        }

        frames = 0;

        curr_ticks_fps = ticks_atm;
    }

    if ((ticks_atm >= curr_ticks_enemy + enemy_wait) && enemies_spawn) {
        CreateEnemy();
        curr_ticks_enemy = SDL_GetTicks();
    }

    SDL_memset(pixels, 0, sizeof(Color)*WIDTH*HEIGHT);

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

    // Update Lasers
    unsigned char dead_laser = 0;
    for (int i = 0; i < laser_count; i++) {
        current_laser->x += current_laser->vx;
        current_laser->y += current_laser->vy;

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
        if (dead_laser || current_laser->x + current_laser->vx < 0 || current_laser->x + current_laser->vx >= WIDTH || current_laser->y + current_laser->vy < 0 || current_laser->y + current_laser->vy >= HEIGHT) {
            KillCurrentLaser();
            SDL_Log("Laser Count: %d", laser_count);
        }

        if (current_laser && current_laser->next_laser) {
            current_laser = current_laser->next_laser;
        }
    }

    // Kill any enemies that have been hit. This loop may become bugged, who knows.
    for (int i = 0; i < enemy_count; i++) {
        if (current_enemy->hit) {
            KillCurrentEnemy();
        }

        if (current_enemy && current_enemy->next_enemy) {
            current_enemy = current_enemy->next_enemy;
        }
    }

    // Update enemies. Note that this, as-is, will prevent the player from dying as well. Not sure if that's something we want to change in the future or anything
    if (enemies_move) {
        for (int i = 0; i < enemy_count; i++) {
            current_enemy->vx = enemy_speed*(position[0] - current_enemy->x)/SDL_sqrt(SDL_pow(position[0] - current_enemy->x, 2) + SDL_pow(position[1] - current_enemy->y, 2));
            current_enemy->vy = enemy_speed*(position[1] - current_enemy->y)/SDL_sqrt(SDL_pow(position[0] - current_enemy->x, 2) + SDL_pow(position[1] - current_enemy->y, 2));

            current_enemy->x += current_enemy->vx;
            current_enemy->y += current_enemy->vy; // No need to store vx and vy tbh

            // Determine if player should be dead
            if ((position[0] + 10 >= current_enemy->x - current_enemy->length && position[1] + 10 >= current_enemy->y - current_enemy->length) && (position[0] - 10 < current_enemy->x + current_enemy->length && position[1] - 10 < current_enemy->y + current_enemy->length)) {
                RenderString("GAME OVER", 10, WIDTH/2 - 8*10/2, HEIGHT/2 - 8*10/2, (Color){255, 255, 255, 255}); // Using 8*10/2 to emphasize 8x8 bits, 10 chars wide, split in half to offset.
                paused = 1;

                SDL_Log("DEAD");
            }

            current_enemy = current_enemy->next_enemy;
        }
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

    // Draw Lasers
    for (int i = 0; i < laser_count; i++) {
        float ix = current_laser->x;
        float iy = current_laser->y;

        for (int j = 0; j < SDL_ceil(current_laser->length); j++) {
            // This (int)iy (int)ix thing is so weird and was a bitch-and-a-half to figure out
            pixels[(int)iy*WIDTH + (int)ix] = current_laser->color;

            ix += current_laser->vx/current_laser->length;
            iy += current_laser->vy/current_laser->length;
        }

        current_laser = current_laser->next_laser;
    }
    
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

    frames++;
    return 0;
}
