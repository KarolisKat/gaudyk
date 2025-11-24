#include "Utils.h"
#include "Objects.h"

bool utils::IsBetween(float val, float rangeB, float rangeE)
{
    return val > rangeB && val < rangeE;
}

bool utils::InOnPlate(Player& player, Plate& plate)
{
    bool betweenX = (player.LegsStartX() < plate.EndX()) && (player.LegsEndX() > plate.StartX());
    bool betweenY = (player.y >= plate.TopY()) && (player.y <= plate.BottomY());
    return betweenX && betweenY;
}
