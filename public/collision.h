#ifndef COLLISION_H
#define COLLISION_H

// in time this should return more involved intersection
// information.

// for now I'm only using yes/no hit tests.

struct point {
    float x;
    float y;
};
static struct point point(float x, float y);

struct circle {
    float x;
    float y;
    float radius;
};
static struct circle circle(float x, float y, float radius);

// these rectangles are not centered
struct rectangle {
    float x;
    float y;
    float w;
    float h;
};
static struct rectangle rectangle(float x, float y, float w, float h);
static struct rectangle rectangle_centered(float center_x, float center_y, float half_width, float half_height);

static bool rectangle_rectangle_intersection(struct rectangle a, struct rectangle b);
static bool rectangle_circle_intersection(struct rectangle a, struct circle b);
static bool circle_circle_intersection(struct circle a, struct circle b);

static bool rectangle_point_intersection(struct rectangle a, struct point b);
static bool circle_point_intersection(struct circle a, struct point b);
static bool point_point_intersection(struct point a, struct point b);

enum shape_type {
    SHAPE_TYPE_POINT, // A point is not technically a shape, but this is for multiple dispatch.
    SHAPE_TYPE_RECTANGLE,
    SHAPE_TYPE_CIRCLE,
    SHAPE_TYPE_COUNT,
};
// While you could construct these from the original shapes,
// this interface makes more sense to use as it's more redundant to inplace construct the namesake
// object. As opposed to building it from arguments like this.
static struct shape shape_circle(float x, float y, float radius);
static struct shape shape_rectangle(float x, float y, float w, float h);
static struct shape shape_rectangle_centered(float center_x, float center_y, float half_width, float half_height);
static struct shape shape_point(float x, float y);
struct shape {
    uint8_t type;
    union {
        struct point     point;
        struct circle    circle;
        struct rectangle rectangle;
    };
};
static bool shape_shape_intersection(struct shape a, struct shape b);

#include "collision.c"
#endif
