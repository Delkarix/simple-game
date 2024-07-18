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

#ifdef USE_OPENCL
    //objects = SDL_malloc(sizeof(Movable)*256);
    FILE* fp = fopen("kernel.cl", "r"); // CHANGE THIS TO USE SDL ROUTINES
    if (!fp) {
        SDL_Log("Failed to load kernel.");
        return -1;
    }

    char* source_str = SDL_malloc(0x1000);
    size_t source_size = fread(source_str, 1, 0x1000, fp); // CHANGE THIS TO USE SDL ROUTINES
    fclose(fp);

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    if (!ret) {
        SDL_Log("Detected platform id: %d", platform_id);
    }
    else {
        SDL_Log("Could not detect the platform. Error code: %d", ret);
        return -1;
    }

    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
    if (!ret) {
        SDL_Log("Detected device id: %d", device_id);
    }
    else {
        SDL_Log("Could not detect the device. Error code: %d", ret);
        return -1;
    }
 
    // Create an OpenCL context
    context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
    if (!ret) {
        SDL_Log("Successfully created a context");
    }
    else {
        SDL_Log("Failed to create a context. Error code: %d", ret);
        return -1;
    }

 
    // Create a command queue
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (!ret) {
        SDL_Log("Created a command queue");
    }
    else {
        SDL_Log("Failed to create a command queue. Error code: %d", ret);
        return -1;
    }

    movable_objects = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(Movable)*256, NULL, &ret);
    if (!ret) {
        SDL_Log("Created a buffer for the movable objects");
    }
    else {
        SDL_Log("Failed to create a buffer for the movable objects. Error code: %d", ret);
        return -1;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    if (!ret) {
        SDL_Log("Created the program");
    }
    else {
        SDL_Log("Failed to create the program. Error code: %d", ret);
        return -1;
    }
 
    // Build the program
    char build_options[64];
    snprintf(build_options, 64, "-D WIDTH=%d -D HEIGHT=%d", WIDTH, HEIGHT); // Format the macros
    ret = clBuildProgram(program, 1, &device_id, build_options, NULL, NULL);
    if (!ret) {
        SDL_Log("Successfully built the program");
    }
    else {
        SDL_Log("Failed to build the program. Error code: %d", ret);
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        // Allocate memory for the log
        char *log = SDL_malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        SDL_Log("%s", log);
        SDL_free(log);
        return -1;
    }
 
    // Create the OpenCL kernel
    kernel = clCreateKernel(program, "update_pos", &ret);
    if (!ret) {
        SDL_Log("Created the update_pos kernel");
    }
    else {
        SDL_Log("Failed to create the update_pos kernel. Error code: %d", ret);
        return -1;
    }

#endif

    SDL_LockSurface(win_surface);

    pixels = win_surface->pixels; // Create an alias to the win_surface
#endif
    // Create more enemies
    /*for (int i = 0; i < 5; i++) {
        CreateEnemy();
    }*/

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
