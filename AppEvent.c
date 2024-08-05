#include "main.h"

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
    if (!running) {
        return 1;
    }
    
    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case SDLK_Q:
                    // If Q is pressed, set the program's state to "not running", ensuring that everything gets closed out of (including threads, should we have any)
                    running = 0;
                    break;
                case SDLK_W:
                    key_table.w_down = 1;
                    break;
                case SDLK_A:
                    key_table.a_down = 1;
                    break;
                case SDLK_S:
                    key_table.s_down = 1;
                    break;
                case SDLK_D:
                    key_table.d_down = 1;
                    break;
                case SDLK_1:
                    enemies_move = !enemies_move;
                    break;
                case SDLK_2:
                    enemies_spawn = !enemies_spawn;
                    break;
                case SDLK_ESCAPE:
                    paused = !paused;
                    break;
            }
            break;

        case SDL_EVENT_KEY_UP:
            switch (event->key.key) {
                case SDLK_W:
                    key_table.w_down = 0;
                    break;
                case SDLK_A:
                    key_table.a_down = 0;
                    break;
                case SDLK_S:
                    key_table.s_down = 0;
                    break;
                case SDLK_D:
                    key_table.d_down = 0;
                    break;
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            mouse_position[0] = event->motion.x;
            mouse_position[1] = event->motion.y;
            break;
        
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            CreateLaser();
            SDL_Log("Laser Count: %d", laser_count);
            //SDL_Log("Laser velocity: %f", current_laser->vx);
            break;
    }

    return 0;
}
