#include <stdio.h>
#include <math.h>
#include "graphics.h"
#include "geometry.h"

List* memory_pool;

/*
// edges is a buffer containing all of the edges
// maybe update this to be multithreaded?
void updateET(EdgeTable* edge_table, VertexBuffer* vb, Edge* edges, uint16_t edge_count) {
    // Zero-out all edge tables
    for (int i = 0; i < edge_table->height; i++) {
        edge_table->entries[i]->length = 0;
    }

    for (int i = 0; i < edge_count; i++) {
        uint16_t max_y = (int)MAX(0, MIN(vb->data[edges->A_index].y, vb->data[edges->B_index].y));

        if (max_y < edge_table->height) {
            insertIntoList(&edge_table->entries[max_y], i);
        }
    }
}

void addEntriesToAE(EdgeTable* table, ActiveEdge* edges, uint16_t y) {
    for (int i = 0; i < table->entries[y]->length; i++) {
        table->entries[y]->elements[i]
    }
}

// For every line, this adds any new edges per the current y
// POTENTIAL OPTIMIZATION: Maybe sort by X values which make addition/removal quicker
void updateActiveEdges(EdgeTable* table, uint16_t y, ActiveEdge* edges) {
    ActiveEdge* iter = edges;

    // Remove all edges that are no longer relevant (i.e. past valid y)
    while (iter != NULL) {
        if (iter->end_y >= y) {
            *iter = iter->next;
            iter->
        }
    }
}*/

void updateActiveEdges(VertexBuffer* vb, EdgeTable* table, Edge* edges, uint16_t edge_count, uint16_t y) {
    table->entries[y]->length = 0;

    for (int i = 0; i < edge_count; i++) {
        Edge* edge = &edges[i];
        Point a = vb->data[edge->A_index];
        Point b = vb->data[edge->B_index];

        // NOTE: THE a.y != b.y SHOULD BE EVALUATED GENERALLY PRIOR BUT THIS SAVES US TIME LATER + MEMORY + COMPLEXITY
        if (MIN(a.y, b.y) <= y && y <= MAX(a.y, b.y) && a.y != b.y) {
            insertIntoList(&table->entries[y], i);
        }
    }
}

float scanline(Point a, Point b, uint16_t y) {
    return (y - a.y)/(b.y - a.y);
}

// MUST CALL updateActiveEdges() before this
void softwareRender(VertexBuffer* vb, EdgeTable* table, Edge* edges, uint16_t edge_count, uint16_t y) {
    updateActiveEdges(vb, table, edges, edge_count, y);

    //printf("%d: %d\n", y, table->entries[y]->length);

    for (int i = 0; i < table->entries[y]->length - 1; i++) {
        Edge* edge1 = &edges[table->entries[y]->elements[i]];
        Edge* edge2;

        // Find the corresponding edge
        for (int j = i + 1; j < table->entries[y]->length; j++) {
            edge2 = &edges[table->entries[y]->elements[j]]; // This might bug if it can't find the right edge for whatever reason

            if (edge1->sector_id == edge2->sector_id) {
                break;
            }
        }

        // If the edge pair is outside of proper bounds, skip it.
        if (!pointIsValid(vb, edge1->A_index) && !pointIsValid(vb, edge1->B_index) && !pointIsValid(vb, edge2->A_index) && !pointIsValid(vb, edge2->B_index)) {
            continue;
        }

        float xt_start = 0;
        float yt_start = 0;
        float xt_end = 1;
        float yt_end = 1;

        // Calculate custom intersection points
        switch (edge1->sector_line_type) {
            case 0:
                // s1-s2
                yt_end = scanline(vb->data[edge1->A_index], vb->data[edge1->B_index], y);
                break;
            case 1:
                // e1-e2
                xt_start = 1;
                yt_start = scanline(vb->data[edge1->A_index], vb->data[edge1->B_index], y);            
                break;
            case 2:
                // s1-e1
                xt_start = scanline(vb->data[edge1->A_index], vb->data[edge1->B_index], y);
                break;
            case 3:
                // s2-e2
                xt_start = scanline(vb->data[edge1->A_index], vb->data[edge1->B_index], y); 
                yt_start = 1;
        }

        // Calculate custom intersection points
        switch (edge2->sector_line_type) {
            case 0:
                // s1-s2
                xt_end = 0;
                yt_end = scanline(vb->data[edge2->A_index], vb->data[edge2->B_index], y);
                break;
            case 1:
                // e1-e2
                yt_end = scanline(vb->data[edge2->A_index], vb->data[edge2->B_index], y);            
                break;
            case 2:
                // s1-e1
                xt_end = scanline(vb->data[edge2->A_index], vb->data[edge2->B_index], y);
                yt_end = 0;
                break;
            case 3:
                xt_end = scanline(vb->data[edge2->A_index], vb->data[edge2->B_index], y);
                // s2-e2
        }

        float t1_scanline = scanline(vb->data[edge1->A_index], vb->data[edge1->B_index], y);
        float t2_scanline = scanline(vb->data[edge2->A_index], vb->data[edge2->B_index], y);

        float x1 = vb->data[edge1->A_index].x + (vb->data[edge1->B_index].x - vb->data[edge1->A_index].x)*t1_scanline;
        float x2 = vb->data[edge2->A_index].x + (vb->data[edge2->B_index].x - vb->data[edge2->A_index].x)*t2_scanline;

        // If same or not on screen, no point in continuing
        if (x1 == x2 || MAX(x1, x2) < 0 || MIN(x1, x2) >= vb->camera->target_img->width) {
            break;
        }

        float z1 = vb->data[edge1->A_index].z + (vb->data[edge1->B_index].z - vb->data[edge1->A_index].z)*t1_scanline;
        float z2 = vb->data[edge2->A_index].z + (vb->data[edge2->B_index].z - vb->data[edge2->A_index].z)*t2_scanline;

        // Ensure that the edge is visible
        if ((z1 < vb->camera->zbuffer->min_z && z2 < vb->camera->zbuffer->min_z) || (z1 >= vb->camera->zbuffer->max_z && z2 >= vb->camera->zbuffer->max_z)) {
            break;
        }

        float xt_diff = xt_end - xt_start;
        float yt_diff = yt_end - yt_start;

        uint16_t x = MIN(vb->camera->target_img->width - 1, MAX(0, x1)); // Ensure the x is in bounds
        uint16_t end_x = MIN(vb->camera->target_img->width - 1, MAX(0, x2)); // Ensure the x is in bounds
        //int16_t x_diff = end_x - x;
        
        int8_t signum = fabs(x2 - x1)/(x2 - x1);

        float t = 0;
        for (; x != end_x; x += signum) {
            t = (x - x1)/(x2 - x1);
            float z = z1 + (z2 - z1)*t;

            if (z < vb->camera->zbuffer->min_z || z >= vb->camera->zbuffer->data[y*vb->camera->target_img->width + x]) {
                continue;
            }

            // Z buffering
            vb->camera->zbuffer->data[y*vb->camera->target_img->width + x] = z;

            float t_y = yt_start + yt_diff*t;
            float t_x = xt_start + xt_diff*t;

            if (0 <= t_x && t_x < 1 && 0 <= t_y && t_y < 1) {
                Image* img = vb->textures->images[edge1->texture_index];

                uint16_t img_x = img->width*t_x;
                uint16_t img_y = img->height*((t_y + edge1->sector_id)/(float)edge1->sector_count);

                

                // Write pixel (and do alpha blending)
                if (img_x < img->width && img_y < img->height) {
                    Color* prev_color = &vb->camera->target_img->pixels[y*vb->camera->target_img->width + x];
                    Color new_color = img->pixels[img_y*img->width + img_x];

                    // NOTE: THIS MAY FUCK UP
                    /*float at = new_color.A/255.0;
                    prev_color->R += at*(new_color.R - prev_color->R);
                    prev_color->G += at*(new_color.G - prev_color->G);
                    prev_color->B += at*(new_color.B - prev_color->B);
                    prev_color->A = 255;*/

                    vb->camera->target_img->pixels[y*vb->camera->target_img->width + x] = img->pixels[img_y*img->width + img_x];
                }
            }
        }
    }
}