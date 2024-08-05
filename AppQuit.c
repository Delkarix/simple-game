#include "main.h"

void SDL_AppQuit(void* appstate) {
    SDL_Log("quidding\n");

    running = 0;

#ifndef USE_WINDOW_SURFACE
    if (main_texture) {
        SDL_DestroyTexture(main_texture);
        main_texture = NULL;
    }

    if (pixels) {
        SDL_free(pixels);
        pixels = NULL;
    }
    

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
        
#else
    SDL_UnlockSurface(win_surface);
#endif

#ifdef USE_OPENCL
    // Probably should have testing for each of these
    clFlush(command_queue);
    clFinish(command_queue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(movable_objects);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
#endif
    
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
        
    SDL_Log("freed shit");
}
