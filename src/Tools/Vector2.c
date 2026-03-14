//
// Created by berke on 3/11/2026.
//

#include "../../Headers/Tools/Vector2.h"

#include <math.h>

float vector2_length(const Vector2 vector) {
    return sqrtf(vector.x * vector.x + vector.y * vector.y);
}

float vector2_distance(const Vector2 vector1, const Vector2 vector2) {
    float y = vector1.y - vector2.y;
    float x = vector1.x - vector2.x;
    return sqrtf(x * x + y * y);
}

Vector2 vector2_add(const Vector2 vector1, const Vector2 vector2) {
    Vector2 result = {vector1.x + vector2.x, vector1.y + vector2.y};
    return result;
}
Vector2 vector2_subtract(Vector2 vector1, Vector2 vector2) {
    Vector2 result = {vector1.x - vector2.x, vector1.y - vector2.y};
    return result;
}
Vector2 vector2_normalized(Vector2 vector) {
    float length = vector2_length(vector);
    Vector2 result = {vector.x / length, vector.y / length};
    return result;
}
Vector2 vector2_multiply_with_float(Vector2 vector, float f) {
    Vector2 result = {vector.x * f, vector.y * f};
    return result;
}
