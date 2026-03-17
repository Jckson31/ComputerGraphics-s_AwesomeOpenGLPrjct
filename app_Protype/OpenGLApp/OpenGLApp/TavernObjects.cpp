#include "TavernObjects.h"

// Costruttore base (Cucina)
TavernObject::TavernObject(glm::vec2 startPos, glm::vec2 startSize, glm::vec3 startColor) {
    pos = startPos;
    size = startSize;
    color = startColor;
}

// Costruttore per i tavoli
TavernObject::TavernObject(glm::vec2 startPos, glm::vec2 startSize, glm::vec3 startColor, std::vector<glm::vec2> seats) {
    pos = startPos;
    size = startSize;
    color = startColor;
    seats = seats;
}

glm::vec2 TavernObject::getPos() const { return pos; }
glm::vec2 TavernObject::getSize() const { return size; }
glm::vec3 TavernObject::getColor() const { return color; }
int TavernObject::getSeatCount() const { return seats.size(); }

// Questa funzione è magica: fa il calcolo "centro del tavolo + sedia" al posto del Main!
glm::vec2 TavernObject::getSeatPos(int index) const {
    return pos + seats[index];
}
