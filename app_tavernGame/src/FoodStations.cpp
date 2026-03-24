#include "FoodStations.h"

FoodStation::FoodStation(glm::vec2 startPos, glm::vec2 startSize, int fId) {
    pos = startPos;   //coordinate in cui si trova la portazione sullo schermo
    size = startSize; //larghezza e altezza dell'oggetto
    foodId = fId;     // identificativo del tipo di pietanza 
}

glm::vec2 FoodStation::getPos() const { return pos; }
glm::vec2 FoodStation::getSize() const { return size; }
int FoodStation::getFoodId() const { return foodId; }