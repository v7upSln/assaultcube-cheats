#pragma once
#include <windows.h>

namespace offsets {
    const uintptr_t localPlayer = 0x18AC00;
    const uintptr_t entityList = 0x18AC04;
    const uintptr_t playerCount = 0x18AC0C;
    const uintptr_t viewMatrix = 0x0057DFD0;
    const uintptr_t gameMode = 0x18ABF8;

    const uintptr_t headX = 0x4;
    const uintptr_t headY = 0x8;
    const uintptr_t headZ = 0xC;
    const uintptr_t x = 0x28;
    const uintptr_t y = 0x2C;
    const uintptr_t z = 0x30;
    const uintptr_t yaw = 0x34;
    const uintptr_t pitch = 0x38;
    const uintptr_t health = 0xEC;
    const uintptr_t ammo = 0x140;
    const uintptr_t username = 0x205;
    const uintptr_t team = 0x30C;
    const uintptr_t isDead = 0x318;
}