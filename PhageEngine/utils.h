#ifndef UTILS_H
#define UTILS_H

#include "stdint.h"

#define PI 3.14159265358979
#define TO_RADIANS(x) ((x * PI) / 180.0)

#define ERROR_SUCCESS 0;
#define ERROR_OUT_OF_BOUNDS 1;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

// Rounds the radians over (0, 2*PI). Note that math.h must have been imported.
#define ROUND_RADIANS(angle) (fmod(2*PI + fmod(angle, 2*PI), 2*PI))

// "\x1b[48;2;000;000;000m" + 1 (null character/space)
//#define COLOR_STRING_LENGTH 20

typedef struct List {
    uint16_t capacity;
    uint16_t length;
    uint16_t elements[];
} List;

void insertIntoList(List** list, uint16_t item);
void removeAt(List* list, uint16_t i);

/**
 * Computes the distance between two points.
 * p1: The first point.
 * p2: The second point.
 * Returns: A double representing the distance between the two points.
 */
//double getDistance(point_t p1, point_t p2);

#endif