#pragma once
#include "Defines.h"

struct Player
{
    float LegsStartX() { return x + 24; }
    float LegsEndX() { return x + 64; }
    float LegsY() { return y + 73; }

    float x, y; // vir�utinio kairiojo kampo koordinates
};

struct Plate
{
    float StartX() { return x; }
    float EndX() { return x + PLATES_WIDTH; }
    float TopY() { return y; }
    float BottomY() { return y + PLATES_HEIGHT; }

    float x, y;
    bool counted = false;
    bool active = false; // naujas: ar plok�tel� spawninta ir krenta
};
