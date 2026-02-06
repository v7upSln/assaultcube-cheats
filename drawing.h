#pragma once
#include <windows.h>
#include <cmath>

struct Vector3 {
    float x, y, z;

    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
};

struct Vector2 { float x, y; };

bool WorldToScreen(Vector3 pos, Vector2& screen, float matrix[16], int windowWidth, int windowHeight);