#ifndef VECTOR2_H
#define VECTOR2_H

/*
  stupid C code to do basic vector2d transformations and operations.
  
  Unoptimized, but should be fine to drop-in and use very quickly.
*/

struct matrix3x3 {
    float m11;
    float m12;
    float m13;

    float m21;
    float m22;
    float m23;

    float m31;
    float m32;
    float m33;
};

static struct matrix3x3 matrix3x3_identity = (struct matrix3x3) {
    .m11 = 1,
    .m22 = 1,
    .m33 = 1
};

struct vector2 {
    float x;
    float y;
};

// Interface

static struct vector2 vector2(float x, float y);
static struct vector2 vector2_from_radians(float radians);
static struct vector2 vector2_add(struct vector2 a, struct vector2 b);
static struct vector2 vector2_subtract(struct vector2 a, struct vector2 b);
static struct vector2 vector2_hadamard(struct vector2 a, struct vector2 b);
static struct vector2 vector2_scale(struct vector2 a, float k);
static float          vector2_dot(struct vector2 a, struct vector2 b);
static struct vector2 vector2_project(struct vector2 projecting, struct vector2 projected_on);

static float          vector2_magnitude(struct vector2 v);
static float          vector2_magnitude_squared(struct vector2 v);
static struct vector2 vector2_normalized(struct vector2 v);
static struct vector2 vector2_perpendicular(struct vector2 v);
static struct vector2 vector2_rotate_by_radians(struct vector2 v, float radians);
static struct vector2 vector2_rotate_by_degrees(struct vector2 v, float degrees);

static struct vector2 vector2_direction_between(struct vector2 source, struct vector2 destination);

static float   vector2_distance_between(struct vector2 source, struct vector2 destination);
static float   vector2_distance_squared_between(struct vector2 source, struct vector2 destination);

static float vector2_angle_in_radians_between(struct vector2 a, struct vector2 b);

// This is named so because it isn't a real multiplication.
// This assumes row matrix style transformation IE: vector * matrix
// this will assume a vector of (v.x, v.y, 1)
static struct vector2 vector2_transform_by_matrix3x3(struct vector2 v, struct matrix3x3 m);

#include "vector2d.c"
#endif
