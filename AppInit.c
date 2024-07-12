#include "main.h"

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

    pixels = SDL_malloc(sizeof(Color)*WIDTH*HEIGHT);
    
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

    pixels = win_surface->pixels; // Create an alias to the win_surface
#endif
    // Randomize enemy positions
    for (int i = 0; i < 5; i++) {
        CreateEnemy();
    }

    SDL_ShowWindow(window);
    SDL_Log("starded");
    //SDL_Log("%ld", sizeof(const unsigned long long));

#ifdef TARGET_FPS
    curr_ticks = SDL_GetTicks();
#endif
    curr_ticks_fps = SDL_GetTicks();
    curr_ticks_enemy = SDL_GetTicks();

    return 0;
}
