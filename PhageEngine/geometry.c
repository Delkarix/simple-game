#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "geometry.h"

// This takes a series of polygons and "hashes" their vertices by each X, Y, and Z.
/*void compile(Polygon** polygons, uint16_t polygon_count, VertexBuffer* vb) {
    uint32_t index = 0;

    // Iterate through all polygons and accumulate their common Xs together
    for (int i = 0; i < polygon_count; i++) {
        for (int j = 0; j < polygons[i]->vertices_count; j++) {
            bool found_x = false;

            // Iterate through previous points
            for (int k = 0; k < index; k++) {
                // Determine if the point already exists in the vertex buffer.
                //printf("%ld\n", hash);
                if (polygons[i]->vertices[j].x == vb->data[k]) {
                    // If not, flag to add it.
                    found_x = true;
                    break;
                }
            }

            if (!found_x) {
                vb->data[index++] = polygons[i]->vertices[j].x;
            }
        }
    }

    vb->x_len = index;
    index = 0;

    // Iterate through all polygons and accumulate their common Ys together
    for (int i = 0; i < polygon_count; i++) {
        for (int j = 0; j < polygons[i]->vertices_count; j++) {
            bool found_y = false;

            // Iterate through previous points
            for (int k = vb->x_len; k < vb->x_len + index; k++) {
                // Determine if the point already exists in the vertex buffer.
                //printf("%ld\n", hash);
                if (polygons[i]->vertices[j].y == vb->data[k]) {
                    // If not, flag to add it.
                    found_y = true;
                    break;
                }
            }

            if (!found_y) {
                vb->data[vb->x_len + index++] = polygons[i]->vertices[j].y;
            }
        }
    }

    vb->y_len = index;
    index = 0;

    // Iterate through all polygons and accumulate their common Ys together
    for (int i = 0; i < polygon_count; i++) {
        for (int j = 0; j < polygons[i]->vertices_count; j++) {
            bool found_z = false;

            // Iterate through previous points
            for (int k = vb->x_len + vb->y_len; k < vb->x_len + vb->y_len + index; k++) {
                // Determine if the point already exists in the vertex buffer.
                //printf("%ld\n", hash);
                if (polygons[i]->vertices[j].z == vb->data[k]) {
                    // If not, flag to add it.
                    found_z = true;
                    break;
                }
            }

            if (!found_z) {
                vb->data[vb->x_len + vb->y_len + index++] = polygons[i]->vertices[j].z;
            }
        }
    }

    vb->z_len = index;
}*/

/*
// Shrinks the size of vertices list by 25% and aggregates all data together
void compile(Polygon** polygons, uint16_t polygon_count, VertexBuffer* vb) {
    int index = 0;

    // Aggregate all xs together
    for (int i = 0; i < polygon_count; i++) {
        for (int j = 0; j < polygons[i]->vertices_count; j++) {
            vb->data[index++] = polygons[i]->vertices[j].x;
        }
    }

    vb->length = index;

    // Aggregate all ys together
    for (int i = 0; i < polygon_count; i++) {
        for (int j = 0; j < polygons[i]->vertices_count; j++) {
            vb->data[index++] = polygons[i]->vertices[j].y;
        }
    }

    // Aggregate all zs together
    for (int i = 0; i < polygon_count; i++) {
        for (int j = 0; j < polygons[i]->vertices_count; j++) {
            vb->data[index++] = polygons[i]->vertices[j].z;
        }
    }
}
*/

bool pointsEqual(Point a, Point b) {
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}

void compile(Polygon** polygons, uint16_t polygon_count, VertexBuffer* vb) {
    vb->length = 0;

    for (int i = 0; i < polygon_count; i++) {
        for (int j = 0; j < polygons[i]->vertices_count; j++) {
            if (!pointInVB(vb, polygons[i]->vertices[j])) {
                vb->data[vb->length++] = polygons[i]->vertices[j];
            }
        }

        bool found = false;
        for (int j = 0; j < vb->textures->count; j++) {
            if (polygons[i]->texture == vb->textures->images[j]) {
                found = true;
                break;
            }
        }

        if (!found) {
            vb->textures->images[vb->textures->count++] = polygons[i]->texture;
        }
    }
}

bool pointInVB(VertexBuffer* vb, Point p) {
    for (int k = 0; k < vb->length; k++) {
        if (pointsEqual(p, vb->data[k])) {
            return true;
        }
    }

    return false;
}

// Probably should do something better for hashing, but it doesn't matter too much since this is AOT
// Also this requires that you know the point is in the buffer otherwise it returns -1 which when unsigned equals 0xFFFF which could be a false positive.
uint16_t getPointIndex(VertexBuffer* vb, Point p) {
    for (int k = 0; k < vb->length; k++) {
        if (pointsEqual(p, vb->data[k])) {
            return k;
        }
    }

    return -1;
}

uint16_t getTextureIndex(VertexBuffer* vb, Image* img) {
    for (int k = 0; k < vb->length; k++) {
        if (img == vb->textures->images[k]) {
            return k;
        }
    }

    return -1;
}

// For optimization
bool pointIsValid(VertexBuffer* vb, uint16_t index) {
    Point p = vb->data[index];

    // 1. Ensure that the point is       min_z < p.z < max_z
    // 2. Ensure that the point is       0 <= p.x < width
    // 3. Ensure that the point is       0 <= p.y < height
    return p.z > vb->camera->zbuffer->min_z && p.z < vb->camera->zbuffer->max_z && p.x >= 0 && p.x < vb->camera->target_img->width && p.y >= 0 && p.y < vb->camera->target_img->height;
}

// THIS NEEDS TO BE OPTIMIZED IN THE FUTURE (see note under the start_index while loop)
// MUST HAVE CALLED compile() AT SOME POINT BEFORE CALLING THIS
// Populates sectors, returns number of sectors
uint16_t generateSectors(Polygon** polygons, uint16_t polygon_count, Edge* edge_buf, VertexBuffer* vb) {
    uint16_t edge_index = 0;

    for (int i = 0; i < polygon_count; i++) {
        uint16_t start_index = 0;
        uint16_t end_index = polygons[i]->vertices_count - 1;
        

        /*Edge s1s2;
        Edge e1e2;
        Edge s1e1;
        Edge s2e2;*/

        // Build list of sectors
        while (start_index + 1 <= end_index - 1) {
            // NOTE: DO PROPER HASHING (Compilation) IN THE FUTURE TO AVOID DUPLICATES. PERHAPS DO GROUPS OF {X,Z},{Y,Z} AND HASH THOSE AND DO LINEAR SEARCH FOR IDS
            
            // As an optimization, determine whether to cull a sector if it is not visible.
            //if (pointIsValid(vb, start_index) || pointIsValid(vb, start_index + 1) || pointIsValid(vb, end_index - 1) || pointIsValid(vb, end_index)) {
            uint16_t s1 = getPointIndex(vb, polygons[i]->vertices[start_index]);
            uint16_t s2 = getPointIndex(vb, polygons[i]->vertices[start_index + 1]);
            uint16_t e2 = getPointIndex(vb, polygons[i]->vertices[end_index - 1]);
            uint16_t e1 = getPointIndex(vb, polygons[i]->vertices[end_index]);

            uint16_t tex_index = getTextureIndex(vb, polygons[i]->texture);

            edge_buf[edge_index++] = (Edge){ .A_index = s1, .B_index = s2, .sector_line_type = 0, .sector_id = start_index, .sector_count = ceilf(polygons[i]->vertices_count/2.0) - 1, .texture_index = tex_index }; // s1-s2
            edge_buf[edge_index++] = (Edge){ .A_index = e1, .B_index = e2, .sector_line_type = 1, .sector_id = start_index, .sector_count = ceilf(polygons[i]->vertices_count/2.0) - 1, .texture_index = tex_index }; // e1-e2
            edge_buf[edge_index++] = (Edge){ .A_index = s1, .B_index = e1, .sector_line_type = 2, .sector_id = start_index, .sector_count = ceilf(polygons[i]->vertices_count/2.0) - 1, .texture_index = tex_index }; // s1-e1
            edge_buf[edge_index++] = (Edge){ .A_index = s2, .B_index = e2, .sector_line_type = 3, .sector_id = start_index, .sector_count = ceilf(polygons[i]->vertices_count/2.0) - 1, .texture_index = tex_index}; // s2-e2

                // NOTE: For complex polygons (i.e. more than 1 sector), there will be duplicate edges (but they differ by their sector_line_type)
            //}
            

            /*s1s2 = (Edge){ s1, s2, 0, MIN(s1->y, s2->y), MAX(s1->y, s2->y) };
            e1e2 = (Edge){ e1, e2, 1, MIN(e1->y, e2->y), MAX(e1->y, e2->y) };
            s1e1 = (Edge){ s1, e1, 2, MIN(s1->y, e1->y), MAX(s1->y, e1->y) };
            s2e2 = (Edge){ s2, e2, 3, MIN(s2->y, e2->y), MAX(s2->y, e2->y) };

            sectors->edges[0] = s1s2;
            sectors->edges[1] = e1e2;
            sectors->edges[2] = s1e1;
            sectors->edges[3] = s2e2;*/
            
            /*
            sectors[sector_index].texture_index = polygons[i]->texture_index;
            sectors[sector_index].sector_id = start_index;
            sectors[sector_index].total_sectors = ceilf(polygons[i]->vertices_count/2.0) - 1;
            */

            start_index++;
            end_index--;
            //sector_index++;
        }
    }

    return edge_index;
}

// Use this instead of moving the camera by hand.
void moveCamera(VertexBuffer* vb, float dx, float dy, float dz) {
    vb->camera->x += dx;
    vb->camera->y += dy;
    vb->camera->z += dz;

    // This is easily vectorized so it should be super fast. Maybe even use thread jobs to speed it up even more.
    for (int i = 0; i < vb->length; i++) {
        vb->data[i].x -= dx;
        vb->data[i].y -= dy;
        vb->data[i].z -= dz;
    }
}

// NOTE: MAKE SURE TO CALL moveCamera() ON INITIALIZATION AND AFTER EVERY TIME THE CAMERA MOVES/ROTATES
void project(VertexBuffer* dest, VertexBuffer* src) {
    for (int i = 0; i < src->length; i++) {
        // This is a dot product
        float bx = src->camera->Vx*src->data[i].x;
        float by = src->camera->Vy*src->data[i].y;
        float bz = src->camera->Vz*src->data[i].z;
        float d = bx + by + bz; // It's almost guaranteed that this will not equal 0.

        float xf = (src->camera->Vx*src->data[i].z - src->camera->Vz*src->data[i].x)/sqrtf(src->camera->Vx*src->camera->Vx + src->camera->Vz*src->camera->Vz);
        float yf = (src->camera->Vy*src->data[i].z - src->camera->Vz*src->data[i].y)/sqrtf(src->camera->Vy*src->camera->Vy + src->camera->Vz*src->camera->Vz);

        // PERHAPS CHANGE THIS MECHANISM SO THE DIVISION IS DONE ELSEWHERE
        float x = src->camera->target_img->width/2.0 + src->camera->target_img->width*xf/(2*fabs(d)*tanf(src->camera->x_fov/2));
        float y = src->camera->target_img->height/2.0 - src->camera->target_img->height*yf/(2*fabs(d)*tanf(src->camera->y_fov/2));

        dest->data[i].x = x;
        dest->data[i].y = y;
        dest->data[i].z = d;
    }
}