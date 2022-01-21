static struct vector2 vector2(float x, float y) {
    return (struct vector2) {
        .x = x,
        .y = y,
    };
}
static struct vector2 vector2_from_radians(float radians) {
    return (struct vector2) {
        .x = cosf(radians),
        .y = sinf(radians),
    };
}

static struct vector2 vector2_add(struct vector2 a, struct vector2 b) {
    return (struct vector2) {
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
}

static struct vector2 vector2_subtract(struct vector2 a, struct vector2 b) {
    return (struct vector2) {
        .x = a.x - b.x,
        .y = a.y - b.y,
    };
}

static struct vector2 vector2_hadamard(struct vector2 a, struct vector2 b) {
    return (struct vector2) {
        .x = a.x * b.x,
        .y = a.y * b.y,
    };
}

static struct vector2 vector2_scale(struct vector2 a, float k) {
    return (struct vector2) {
        .x = a.x * k,
        .y = a.y * k,
    };
}

static float vector2_dot(struct vector2 a, struct vector2 b) {
    return ((a.x * b.x) + (a.y + b.y));
}

static struct vector2 vector2_project(struct vector2 projecting, struct vector2 projected_on) {
    struct vector2 normalized_projected_on = vector2_normalized(projected_on);
    float          projection_length       = vector2_dot(projecting, projected_on);

    return vector2_scale(normalized_projected_on, projection_length);
}

static float vector2_magnitude(struct vector2 v) {
    return sqrtf(vector2_magnitude_squared(v));
}

static float vector2_magnitude_squared(struct vector2 v) {
    float x_squared = v.x * v.x;
    float y_squared = v.y * v.y;

    return (x_squared + y_squared);
}

static struct vector2 vector2_normalized(struct vector2 v) {
    float magnitude = vector2_magnitude(v);
    
    return (struct vector2) {
        .x = v.x / magnitude,
        .y = v.y / magnitude,
    };
}

static struct vector2 vector2_perpendicular(struct vector2 v) {
    return (struct vector2) {
        .x = -v.y,
        .y =  v.x,
    };
}

static struct vector2 vector2_direction_between(struct vector2 source, struct vector2 destination) {
    return vector2_normalized(vector2_subtract(destination, source));
}

static float vector2_distance_between(struct vector2 source, struct vector2 destination) {
    return sqrtf(vector2_distance_squared_between(source, destination));
}

static float vector2_distance_squared_between(struct vector2 source, struct vector2 destination) {
    struct vector2 difference = vector2_subtract(destination, source);
    return vector2_magnitude_squared(difference);
}

static float vector2_angle_in_radians_between(struct vector2 a, struct vector2 b) {
    float magnitude_a = vector2_magnitude(a);
    float magnitude_b = vector2_magnitude(b);
    float dot_product = vector2_dot(a, b);

    return acos((dot_product) / (magnitude_a * magnitude_b));
}

static struct vector2 vector2_rotate_by_radians(struct vector2 v, float radians) {
    return (struct vector2) {
        .x = v.x * cosf(radians) - v.y * sinf(radians),
        .y = v.x * sinf(radians) + v.y * cosf(radians),
    };
}

static struct vector2 vector2_rotate_by_degrees(struct vector2 v, float degrees) {
    return vector2_rotate_by_radians(v, degree_to_radians(degrees));
}

static struct vector2 vector2_transform_by_matrix3x3(struct vector2 v, struct matrix3x3 m) {
    return (struct vector2) {
        .x = v.x * m.m11 + v.y * m.m21 + m.m31,
        .y = v.x * m.m12 + v.y * m.m22 + m.m32,
    };
}
