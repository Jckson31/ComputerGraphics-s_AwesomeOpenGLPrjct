#pragma once
#include <glm/glm.hpp>


class Dirt {
private:
    glm::vec2 pos;
    glm::vec2 size;

public:
    // Costruttore: gli passiamo solo la posizione, la grandezza la decide lui!
    Dirt(glm::vec2 startPos);

    // Getters
    glm::vec2 getPos() const;
    glm::vec2 getSize() const;
};
