#include "Dirt.h"

Dirt::Dirt(glm::vec2 startPos) {
    pos = startPos;
    size = glm::vec2(35.0f, 35.0f); // Fissiamo la grandezza standard dello sporco qui!
}

glm::vec2 Dirt::getPos() const { return pos; }
glm::vec2 Dirt::getSize() const { return size; }