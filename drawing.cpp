#include "pch.h"
#include "drawing.h"

bool WorldToScreen(Vector3 pos, Vector2& screen, float matrix[16], int windowWidth, int windowHeight) {
    float clipCoordsX = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
    float clipCoordsY = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
    float clipCoordsW = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

    if (clipCoordsW < 0.1f) return false;

    Vector2 ndc;
    ndc.x = clipCoordsX / clipCoordsW;
    ndc.y = clipCoordsY / clipCoordsW;

    screen.x = (windowWidth / 2.0f * ndc.x) + (ndc.x + windowWidth / 2.0f);
    screen.y = -(windowHeight / 2.0f * ndc.y) + (ndc.y + windowHeight / 2.0f);
    return true;
}