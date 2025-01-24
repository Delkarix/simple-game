#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"
#include "graphics.h"

// Represents a point in 2D space.
typedef struct Point {
    float x; // The X coordinate of the point.
    float y; // The Y coordinate of the point.
    float z; // The Z coorindate of the point.
} Point;

typedef struct Edge {
    uint16_t A_index; // Index of the point in the central vertex buffer
    uint16_t B_index; // Index of the point in the central vertex buffer

    uint8_t sector_line_type;
    uint8_t sector_id;
    uint8_t sector_count;
    uint16_t texture_index;
    
    // Used internally
    //float min_y;
    //float max_y;
} Edge;


typedef struct Polygon {
    Image* texture; // Texture in the central texture buffer.

    uint8_t vertices_count;
    Point vertices[]; // array of indices to central vertex buffer. Flexible array.
} Polygon;

/*typedef struct Sector {
    uint16_t texture_index; // Index (or ID) of the texture in the central texture buffer.
    Edge edges[4]; // There will always be 4 edges per Sector.

    uint8_t sector_id;
    uint8_t total_sectors; // TODO: WE WILL HAVE A SECTOR/POLYGON INFO BUFFER

    // Used internally
    //float min_y;
    //float max_y;
} Sector;*/

/*
typedef struct VertexBuffer {
    uint16_t x_len;
    uint16_t y_len;
    uint16_t z_len;
    float data[];
} VertexBuffer;
*/

typedef struct {
    float x;
    float y;
    float z;

    float Vx; // The direction the camera is looking on the X axis
    float Vy; // The direction the camera is looking on the Y axis
    float Vz; // The direction the camera is looking on the Z axis

    float x_fov;
    float y_fov;

    Image* target_img;
    ZBuffer* zbuffer;
} Camera;

typedef struct VertexBuffer {
    Camera* camera;
    ImageBuffer* textures;
    uint16_t length; // The total number of vertices stored. .data starts at Xs, spans for length, and offset to the Ys by +length and Zs to +2*length respectively.
    Point data[];
} VertexBuffer;

//void compile(Polygon** polygons, uint16_t polygon_count, VertexBuffer* vb);

//double getLength2D(Edge* e);

//double getLength3D(Edge* e);

bool pointInVB(VertexBuffer* vb, Point p);

bool pointIsValid(VertexBuffer* vb, uint16_t index);

void compile(Polygon** polygons, uint16_t polygon_count, VertexBuffer* vb);

uint16_t generateSectors(Polygon** polygons, uint16_t polygon_count, Edge* edge_buf, VertexBuffer* vb);

void moveCamera(VertexBuffer* vb, float dx, float dy, float dz);

void project(VertexBuffer* dest, VertexBuffer* src);

#endif