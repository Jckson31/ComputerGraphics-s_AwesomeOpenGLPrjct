#pragma once
#include <glm/glm.hpp>

class FoodStation {
private:
    glm::vec2 pos;
    glm::vec2 size;
    int foodId; // 0=Birra, 1=Carne, 2=Zuppa, 3=Pane

public:
    FoodStation(glm::vec2 startPos, glm::vec2 startSize, int fId);

    glm::vec2 getPos() const;
    glm::vec2 getSize() const;
    int getFoodId() const;
};

