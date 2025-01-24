#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
#include "utils.h"

typedef struct Color {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
} Color;

typedef struct VertexBuffer VertexBuffer;
//typedef struct EdgeTable EdgeTable;
typedef struct Edge Edge;


// Represents an image.
typedef struct Image {
	uint16_t width; // Represents the width of the texture.
	uint16_t height; // Represents the height of the texture.

	Color pixels[]; // Flexible Array of pixels.
} Image;

typedef struct ImageBuffer {
    uint32_t count;
    Image* images[]; // Flexible array of textures
} ImageBuffer;

typedef struct ZBuffer {
    float min_z;
    float max_z;

    float data[]; // Flexible array
} ZBuffer;

/*
typedef struct EdgeTableEntry {
    uint16_t edge_index; // Flexible array
    struct EdgeTableEntry* next; // linked list
} EdgeTableEntry;*/

typedef struct EdgeTable {
    uint16_t height; // Basically the height of the image to render to (number of Y values)
    List* entries[]; // Y-length long flexible array, array of arrays that list indices for
} EdgeTable;

// typedef struct ActiveEdge {
//     float start_x;
//     float start_z;
//     float slope_x; // Amount to increment the starting x by. dx/dy. Generally 1/m where m = (y2 - y1)/(x2 - x1)
//     float slope_z; // Amount to increment the starting z by. dz/dy. Generally 1/m where m = (y2 - y1)/(z2 - z1)
//     uint16_t end_y; // The Y-value to end at.

//     struct ActiveEdge* next; // Linked list
// } ActiveEdge;

/**
 * Retrieves a pointer to the requested pixel from the specified image.
 *
 * image: A pointer to the image to retrieve a pixel from.
 * x: The x-coordinate of the requested pixel.
 * y: The y-coordinate of the requested pixel.
 * Returns: A pointer to a color_t value in the image representing the pixel. If the X or Y coordinates are out-of-bounds, then an error_pixel is returned.
 */
int getPixel(Image* image, unsigned short x, unsigned short y, Color* color);

/**
 * Sets the requested pixel from the specified image to a specified color_t value.
 *
 * image: A pointer to the image to retrieve a pixel from.
 * x: The x-coordinate of the requested pixel.
 * y: The y-coordinate of the requested pixel.
 * color: The color to set the pixel to.
 * NOTE: If the X or Y coordinates are out-of-bounds, then nothing will happen.
 */
int setPixel(Image* image, unsigned short x, unsigned short y, Color* color);

void softwareRender(VertexBuffer* vb, EdgeTable* table, Edge* edges, uint16_t edge_count, uint16_t y);

#endif