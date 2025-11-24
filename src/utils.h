#pragma once

struct Player;
struct Plate;

namespace utils
{
    bool IsBetween(float val, float rangeB, float rangeE);
    bool InOnPlate(Player& player, Plate& plate);
}
