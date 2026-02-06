#pragma once
#include <windows.h>
#include "drawing.h"

struct WeaponEntry {
    void* vtable;
    int id;
    struct Player* owner;
};

struct Player {
    /* 0x0000 */ void* vtable;
    /* 0x0004 */ Vector3 headPos;
    /* 0x0010 */ char pad_0010[24];
    /* 0x0028 */ Vector3 pos;
    /* 0x0034 */ float yaw;
    /* 0x0038 */ float pitch;
    /* 0x003C */ char pad_003C[168];
    /* 0x00E4 */ int lastVisibleFrame;
    /* 0x00E8 */ char pad_00E8[4];
    /* 0x00EC */ int health;
    /* 0x00F0 */ int armor;
    /* 0x00F4 */ char pad_00F4[40];
    /* 0x11C */ int mag;
    /* 0x120 */ char pad_120[32];
    /* 0x140 */ int ammo;
    /* 0x144 */ char pad_144[193];
    /* 0x205 */ char name[16];
    /* 0x215 */ char pad_215[247];
    /* 0x30C */ int team; // 0 or 1
    /* 0x310 */ char pad_310[8];
    /* 0x318 */ int state; // 0 = Alive
    /* 0x31C */ char pad_31C[32];
    /* 0x33C */ WeaponEntry* knife;
    /* 0x340 */ WeaponEntry* pistol;
    /* 0x344 */ WeaponEntry* carbine;
    /* 0x348 */ WeaponEntry* shotgun;
    /* 0x34C */ WeaponEntry* subgun;
    /* 0x350 */ WeaponEntry* sniper;
    /* 0x354 */ WeaponEntry* assault;
    /* 0x358 */ WeaponEntry* grenade;
    /* 0x35C */ WeaponEntry* akimbo;
    /* 0x360 */ WeaponEntry* currentWeapon;
};