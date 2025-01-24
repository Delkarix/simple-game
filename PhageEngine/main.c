#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "graphics.h"
//#include "gpu.h"
#include "geometry.h"

#define WIDTH 640
#define HEIGHT 480

static bool finish = false;

Image img1 = {
    .width = 2,
    .height = 2,
    .pixels = {
        {0, 0, 255, 255},
        {0, 255, 0, 255},
        {0, 255, 255, 255},
        {255, 0, 0, 255}
    }
};

Image img2 = {
    .width = 2,
    .height = 2,
    .pixels = {
        {127, 0, 255, 255},
        {0, 127, 0, 255},
        {0, 255, 127, 255},
        {255, 86, 0, 255}
    }
};

Image img3 = {
    .width = 2,
    .height = 2,
    .pixels = {
        {52, 83, 25, 255},
        {1, 78, 0, 255},
        {0, 255, 255, 255},
        {255, 50, 0, 255}
    }
};

Polygon polygon1 = {
    .texture = &img1,
    .vertices_count = 6,
    .vertices = {
        {-2, 2, 2},
        {-4, 0, 2},
        {-2, -2, 2},
        {2, -2, 2},
        {4, 0, 2},
        {2, 2, 2}
    }
};

Polygon polygon2 = {
    .texture = &img2,
    .vertices_count = 4,
    .vertices = {
        {-2, 4, 4},
        {-2, 2, 2},
        {2, 2, 2},
        {2, 4, 4}
    }
};

Polygon polygon3 = {
    .texture = &img3,
    .vertices_count = 4,
    .vertices = {
        {-2, -4, 4},
        {-2, -2, 2},
        {2, -2, 2},
        {2, -4, 4}
    }
};


int main() {
    Polygon* polygon_buffer[] = {&polygon1, &polygon2, &polygon3};

    // These ought to get aggregated (and reallocated)
    VertexBuffer* vb = malloc(0x1000);
    memset(vb, 0, 0x1000);

    vb->textures = malloc(0x1000);
    memset(vb->textures, 0, 0x1000);
    

    VertexBuffer* vb2 = malloc(0x1000);
    memset(vb2, 0, 0x1000);

    Edge* edges = malloc(0x1000);
    memset(edges, 0, 0x1000);

    // DO SCANLINE RENDERING NEXT

    Camera c = {
        .x = 0,
        .y = 0,
        .z = 0,

        .Vx = 0,
        .Vy = 0,
        .Vz = 1,

        .x_fov = TO_RADIANS(60),
        .y_fov = TO_RADIANS(60),

        .target_img = malloc(sizeof(Image) + sizeof(Color)*WIDTH*HEIGHT),
        .zbuffer = malloc(sizeof(ZBuffer) + sizeof(float)*WIDTH*HEIGHT)
    };
    c.target_img->width = WIDTH;
    c.target_img->height = HEIGHT;
    c.zbuffer->min_z = 0;
    c.zbuffer->max_z = 100;
    memset(c.target_img->pixels, 0, sizeof(Color)*WIDTH*HEIGHT);

    vb->camera = &c;

    EdgeTable* et = malloc(0x10000);
    et->height = HEIGHT;
    
    // Populate et
    for (int i = 0; i < HEIGHT; i++) {
        et->entries[i] = malloc(sizeof(List) + sizeof(uint16_t));
        et->entries[i]->capacity = 1;
        et->entries[i]->length = 0;
    }



    // Coalesce the vertices together for faster transformation
    compile(polygon_buffer, sizeof(polygon_buffer)/sizeof(Polygon*), vb);
    uint16_t edge_count = generateSectors(polygon_buffer, sizeof(polygon_buffer)/sizeof(Polygon*), edges, vb);
    printf("VB Size: %d\nEdge count: %d\n", vb->length, edge_count);
    memcpy(vb2, vb, sizeof(vb) + vb->length*sizeof(Point));

    moveCamera(vb2, 0, 0, -10);

    
    

    //memcpy(vb2->data, vb->data, vb->length*sizeof(Point));
    //project(vb);
    //printf("%f\n", vb->data[0].y);

        // Initialize SDL and test for success
    // Initialize SDL and test for success
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("[ERROR SDL_INIT_VIDEO] %s\n", SDL_GetError());
        exit(1);
    }

    // Create window and test for success
    SDL_Window* window = SDL_CreateWindow("SDL Test", WIDTH, HEIGHT, 0);
    if (!window) {
        printf("[ERROR SDL_CreateWindow()] %s\n", SDL_GetError());
        exit(1);
    }

    SDL_WarpMouseInWindow(window, WIDTH/2, HEIGHT/2);

    // Set relative mouse mode. This will lock the mouse inplace and hide it but allow only relative mouse movements to be reported to the event system.
    // This function is a GODSEND for FPS games.
    SDL_SetWindowRelativeMouseMode(window, true);

    // Access the GPU device
    SDL_GPUDevice* gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, 0, NULL);
    if (!gpu) {
        printf("[ERROR SDL_CreateGPUDevice()] %s\n", SDL_GetError());
        exit(1);
    }

    // Claim ownership over the window so we can get its texture
    if (!SDL_ClaimWindowForGPUDevice(gpu, window)) {
        printf("[ERROR SDL_ClaimWindowForGPUDevice()] %s\n", SDL_GetError());
        exit(1);
    }

    if (!SDL_SetGPUSwapchainParameters(gpu, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX)) {
        printf("[ERROR SDL_SetGPUSwapchainParameters()] %s\n", SDL_GetError());
        exit(1);
    }

    // Create a transfer buffer. This will be used to throw pixels in at our window.
    SDL_GPUTransferBufferCreateInfo tb_create_info = {.size = WIDTH*HEIGHT*sizeof(Color), .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD};
    // SDL_GPUTransferBufferCreateInfo tb_create_info = {.size = 4, .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD};
    SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(gpu, &tb_create_info);
    if (!transfer_buffer) {
        printf("[ERROR SDL_CreateGPUTransferBuffer()] %s\n", SDL_GetError());
        exit(1);
    }


    int curr_ticks_fps = SDL_GetTicks();
    int fps = 0;
    int frames = 0;

    int total_ticks = 0;

    int ticks_start = SDL_GetTicks();
    int pause = 0;

    // Continuously listen for events.
    SDL_Event event;
    float ang = PI/180;
    while (!finish) {
        int ticks_atm = SDL_GetTicks();
        if (ticks_atm >= curr_ticks_fps + 1000) {
            fps = frames;
            frames = 0;
            curr_ticks_fps = ticks_atm;

            printf("FPS: %d\n", fps);
        }

        //rotateX(points, polygon2.vertices_count, (Point){0, 0}, ang);
        if (!(rand() % 10) && !pause) {
            //points[0].y += 1;
            //points[rand()%(polygon2.vertices_count)].x += 0.5;
            //points[rand()%(polygon2.vertices_count)].y += 0.5;
            //points[rand()%(polygon2.vertices_count)].z += 0.5;
            //rotateX(points, polygon2.vertices_count, (Point){225, 250, 5}, ang);
            //rotateY(points, polygon2.vertices_count, (Point){225, 250, 5}, ang);
            //rotateZ(points, polygon2.vertices_count, (Point){225, 250, 5}, ang);
            //points[1].z -= 0.5;

		    //ang += PI/180;
        }
		
        
        
        // Continuously process all events
                    float rx = 0;
            float ry = 0;
            float vx;
            float vy;
            float vz;
            

        while (SDL_PollEvent(&event)) {
            // Test for the different types of events
            switch (event.type) {
                // Test for keypresses
                case SDL_EVENT_KEY_DOWN:
                    // Process keypresses
                    switch (event.key.scancode) {
                        case SDL_SCANCODE_Q:
                            finish = 1;
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            pause = !pause;
                            break;
                        case SDL_SCANCODE_W:
                            moveCamera(vb2, c.Vx*0.1, c.Vy*0.1, c.Vz*0.1);
                            break;
                        case SDL_SCANCODE_A:
                            moveCamera(vb2, c.Vz*0.1, 0, -c.Vx*0.1);
                            break;
                        case SDL_SCANCODE_S:
                            moveCamera(vb2, -c.Vx*0.1, -c.Vy*0.1, -c.Vz*0.1);
                            break;
                        case SDL_SCANCODE_D:
                            moveCamera(vb2, -c.Vz*0.1, 0, c.Vx*0.1);
                            break;
                    }

                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    ry += event.motion.xrel*0.0005;
                    rx += event.motion.yrel*0.0005;
                    //printf("%f\n", event.motion.y);
            }
        }

                // Main shit

        vx = c.Vx*SDL_cosf(ry) - c.Vz*SDL_sinf(ry);
        vy = (c.Vz*SDL_cosf(ry) + c.Vx*SDL_sinf(ry))*SDL_sinf(rx) + c.Vy*SDL_cosf(rx);
        vz = (c.Vz*SDL_cosf(ry) + c.Vx*SDL_sinf(ry))*SDL_cosf(rx) - c.Vy*SDL_sinf(rx);

        c.Vx = vx;
        c.Vy = vy;
        c.Vz = vz;

        //memcpy(vb->data, vb2->data, vb2->length*sizeof(Point));
        project(vb, vb2);

        memset(vb->camera->target_img->pixels, 0, sizeof(Color)*WIDTH*HEIGHT);
        // Reset Z buffer
        for (int i = 0; i < WIDTH*HEIGHT; i++) {
            c.zbuffer->data[i] = c.zbuffer->max_z;
        }


        for (int y = 0; y < HEIGHT; y++) {
            softwareRender(vb, et, edges, edge_count, y);
        }

        //memset(camera_image->pixels, 0, sizeof(RGBA)*WIDTH*HEIGHT);

        // Create a new command buffer
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu);
        if (!command_buffer) {
            printf("[ERROR SDL_AcquireGPUCommandBuffer()] %s\n", SDL_GetError());
            exit(1);
        }

        SDL_GPUTexture* texture;

        if (!SDL_AcquireGPUSwapchainTexture(command_buffer, window, &texture, NULL, NULL)) {
            printf("[ERROR SDL_AcquireGPUSwapchainTexture()] %s\n", SDL_GetError());
            exit(1);
        }

        // /*
        //renderSectorsToImage(camera_image, z_buffer, template_z_buffer, sectors, sector_count);
        char* pixels = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, 0);
            if (!pixels) {
                printf("[ERROR SDL_MapGPUTransferBuffer()] %s\n", SDL_GetError());
                exit(1);
            }
            // for (int i = 0; i < WIDTH*HEIGHT*4; i++) {
                // pixels[i] = rand()%256;
            // }
            /*unsigned short rand_1 = (rand() << 16) + rand(); 
            for (int i = 0; i < 256; i++) {
                ((unsigned int*)pixels)[i*WIDTH + i] = rand_1;
            }*/
           memcpy(pixels, c.target_img->pixels, sizeof(Color)*WIDTH*HEIGHT);
           //memset(&pixels[WIDTH*sizeof(RGBA)*225], 255, WIDTH*sizeof(RGBA));
           //memset(&pixels[WIDTH*sizeof(RGBA)*250], 127, WIDTH*sizeof(RGBA));
        SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);
        // */

        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
            if (!copy_pass) {
                printf("[ERROR SDL_BeginGPUCopyPass()] %s\n", SDL_GetError());
                exit(1);
            }


            SDL_GPUTransferBufferLocation trans_loc = {.transfer_buffer = transfer_buffer, .offset = 0};
            /*for (int i = 0; i < 3; i++) {
                    float* col = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, 1);
                        if (!col) {
                            printf("[ERROR SDL_MapGPUTransferBuffer()] %s\n", SDL_GetError());
                            exit(1);
                        }
                        *col = (rand()%256)/256.0;
                    SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);
                    
                    SDL_GPUBufferRegion buf_reg = {.buffer = buffers[i], .offset = 0, .size = 4};

                    SDL_UploadToGPUBuffer(copy_pass, &trans_loc, &buf_reg, 1);
            }*/

            SDL_GPUTextureRegion tex_reg = {.texture = texture, .mip_level = 0, .layer = 0, .x = 0, .y = 0, .z = 0, .w = WIDTH, .h = HEIGHT, .d = 1};

            SDL_GPUTextureTransferInfo trans_info = {.transfer_buffer = transfer_buffer, .pixels_per_row = WIDTH, .rows_per_layer = 0, .offset = 0};
            SDL_UploadToGPUTexture(copy_pass, &trans_info, &tex_reg, 0);

        // End the copy
        SDL_EndGPUCopyPass(copy_pass);

        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(command_buffer);
        // if (!SDL_SubmitGPUCommandBuffer(command_buffer)) {
        if (!fence) {
            printf("[ERROR SDL_SubmitGPUCommandBuffer()] %s\n", SDL_GetError());
            //break;
        }

        int ticks_now = SDL_GetTicks();

        while (!SDL_QueryGPUFence(gpu, fence));

        //SDL_WaitForGPUFences(gpu, 0, &fence, 1);
        total_ticks += SDL_GetTicks() - ticks_now;
        SDL_ReleaseGPUFence(gpu, fence);

        frames++;
    }


    int total = SDL_GetTicks() - ticks_start;
    printf("Ticks Total: %d\nWasted: %d\nPercent: %.3f\n", total, total_ticks, (float)total_ticks * 100 / total);

    SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
    // SDL_ReleaseGPUBuffer(gpu, buffers[2]);
    // SDL_ReleaseGPUBuffer(gpu, buffers[1]);
    // SDL_ReleaseGPUBuffer(gpu, buffers[0]);
    SDL_ReleaseWindowFromGPUDevice(gpu, window);
    SDL_DestroyGPUDevice(gpu);
    SDL_DestroyWindow(window);




    //compile(polygon_buffer, sizeof(polygon_buffer)/sizeof(Polygon*), vb);

    //printf("VB X Size: %d\nVB Y Size: %d\nVB Z Size: %d\n", vb->x_len, vb->y_len, vb->z_len);
    
    free(vb->textures);
    free(et);
    free(vb2);
    free(c.zbuffer);
    free(c.target_img);
    free(edges);
    free(vb);

    // Quit program (uninitialize SDL)
    SDL_Quit();
}
