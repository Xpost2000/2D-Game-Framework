// "construction" like functions.
// These are a new addition and part of the new style.
// I will make these for more obvious structs in the future.
// so that way I don't have to use designated initializers.

// Stylistically I would prefer designated initializers,
// but the game code should be as high level as I can afford and this is one of those cases.

// NOTE(jerry): none of this was tested. Please test this, and remove this once this is verified to be working
static struct point point(float x, float y) {
    return (struct point) {
        .x = x,
        .y = y,
    };
};
static struct circle circle(float x, float y, float radius) {
    return (struct circle) {
        .x      = x,
        .y      = y,
        .radius = radius
    };
}
static struct rectangle rectangle(float x, float y, float w, float h) {
    return (struct rectangle) {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
}
static struct rectangle rectangle_centered(float center_x, float center_y, float half_width, float half_height) {
    return (struct rectangle) {
        .x = center_x - half_width,
        .y = center_y - half_height,
        .w = half_width * 2,
        .h = half_height * 2,
    };
}

// assume vector2d.h was included.
// I don't include here, because this project is a unity build.
static bool rectangle_rectangle_intersection(struct rectangle a, struct rectangle b) {
    struct vector2 first_rectangle_minimum = vector2(a.x, a.y);
    struct vector2 first_rectangle_maximum = vector2(a.x + a.w, a.y + a.h);

    struct vector2 second_rectangle_minimum = vector2(b.x, b.y);
    struct vector2 second_rectangle_maximum = vector2(b.x + b.w, b.y + b.h);
    
    bool x_overlap = (first_rectangle_minimum.x <= second_rectangle_maximum.x && first_rectangle_maximum.x >= second_rectangle_minimum.x);
    bool y_overlap = (first_rectangle_minimum.y <= second_rectangle_maximum.y && first_rectangle_maximum.y >= second_rectangle_minimum.y);

    return (x_overlap && y_overlap);
}

static bool rectangle_circle_intersection(struct rectangle a, struct circle b) {
    struct vector2 rectangle_minimum = vector2(a.x, a.y);
    struct vector2 rectangle_maximum = vector2(a.x+a.w, a.y+a.h);

    
    struct vector2 rectangle_center_position = vector2(a.x + a.w/2, a.y + a.h/2);
    struct vector2 circle_position           = vector2(b.x, b.y);
    struct vector2 position_delta            = vector2_subtract(circle_position, rectangle_center_position);

    struct vector2 closest_point_to_circle = vector2(clamp_float(position_delta.x, rectangle_minimum.x, rectangle_maximum.x),
                                                     clamp_float(position_delta.y, rectangle_minimum.y, rectangle_maximum.y));

    return circle_point_intersection(b, point(closest_point_to_circle.x, closest_point_to_circle.y));
}

static bool rectangle_point_intersection(struct rectangle a, struct point b) {
    struct vector2 rectangle_minimum = vector2(a.x, a.y);
    struct vector2 rectangle_maximum = vector2(a.x+a.w, a.y+a.h);

    bool x_overlap = (b.x <= rectangle_maximum.x && b.x >= rectangle_minimum.x);
    bool y_overlap = (b.y <= rectangle_maximum.y && b.y >= rectangle_minimum.y);

    return (x_overlap && y_overlap);
}

static bool point_point_intersection(struct point a, struct point b) {
    return (a.x == b.x && a.y == b.y);
}

static bool circle_circle_intersection(struct circle a, struct circle b) {
    float sum_of_radi      = a.radius + b.radius;
    float distance_squared = vector2_distance_squared_between(vector2(a.x, a.y), vector2(b.x, b.y));

    return (distance_squared) <= sum_of_radi*sum_of_radi;
}

static bool circle_point_intersection(struct circle a, struct point b) {
    float distance_squared = vector2_distance_squared_between(vector2(a.x, a.y), vector2(b.x, b.y));

    return (distance_squared) <= a.radius*a.radius;
}

static struct shape shape_circle(float x, float y, float radius) {
    return (struct shape) {
        .type   = SHAPE_TYPE_CIRCLE,
        .circle = circle(x, y, radius)
    };
}

static struct shape shape_rectangle_centered(float center_x, float center_y, float half_width, float half_height) {
    return (struct shape) {
        .type = SHAPE_TYPE_RECTANGLE,
        .rectangle = rectangle_centered(center_x, center_y, half_width, half_height)
    };
}
static struct shape shape_rectangle(float x, float y, float w, float h) {
    return (struct shape) {
        .type = SHAPE_TYPE_RECTANGLE,
        .rectangle = rectangle(x, y, w, h)
    };
}
static struct shape shape_point(float x, float y) {
    return (struct shape) {
        .type = SHAPE_TYPE_POINT,
        .point = point(x, y)
    };
}

// crappy C multiple dispatch, wishing for common lisp.
static bool shape_shape_intersection(struct shape a, struct shape b) {
    switch (a.type) {
        case SHAPE_TYPE_POINT: {
            switch (b.type) {
                case SHAPE_TYPE_POINT: {
                    return point_point_intersection(a.point, b.point);
                } break;
                case SHAPE_TYPE_RECTANGLE: {
                    return rectangle_point_intersection(b.rectangle, a.point);
                } break;
                case SHAPE_TYPE_CIRCLE: {
                    return circle_point_intersection(b.circle, a.point);
                } break;
            }
        } break;

        case SHAPE_TYPE_RECTANGLE: {
            switch (b.type) {
                case SHAPE_TYPE_POINT: {
                    return rectangle_point_intersection(a.rectangle, b.point);
                } break;
                case SHAPE_TYPE_RECTANGLE: {
                    return rectangle_rectangle_intersection(a.rectangle, b.rectangle);
                } break;
                case SHAPE_TYPE_CIRCLE: {
                    return rectangle_circle_intersection(a.rectangle, b.circle);
                } break;
            }
        } break;

        case SHAPE_TYPE_CIRCLE: {
            switch (b.type) {
                case SHAPE_TYPE_POINT: {
                    return circle_point_intersection(a.circle, b.point);
                } break;
                case SHAPE_TYPE_RECTANGLE: {
                    return rectangle_circle_intersection(b.rectangle, a.circle);
                } break;
                case SHAPE_TYPE_CIRCLE: {
                    return circle_circle_intersection(a.circle, b.circle);
                } break;
            }
        } break;
    }
    unreachable_code();   
    return false;
}
