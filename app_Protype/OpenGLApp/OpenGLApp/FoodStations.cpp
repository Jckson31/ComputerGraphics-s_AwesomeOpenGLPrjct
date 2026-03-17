#include "FoodStations.h"

FoodStation::FoodStation(glm::vec2 startPos, glm::vec2 startSize, int fId) {
    pos = startPos;
    size = startSize;
    foodId = fId;
}

glm::vec2 FoodStation::getPos() const { return pos; }
glm::vec2 FoodStation::getSize() const { return size; }
int FoodStation::getFoodId() const { return foodId; }