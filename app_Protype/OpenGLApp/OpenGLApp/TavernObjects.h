#pragma once
#include <glm/glm.hpp>
#include <vector>

class TavernObject {
private:
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec3 color;
    std::vector<glm::vec2> seats; // Le coordinate delle sedie (se è un tavolo)

public:
    // Costruttore 1: Per oggetti semplici senza sedie (es. Il bancone della cucina)
    TavernObject(glm::vec2 startPos, glm::vec2 startSize, glm::vec3 startColor);

    // Costruttore 2: Per i tavoli (che hanno le sedie)
    TavernObject(glm::vec2 startPos, glm::vec2 startSize, glm::vec3 startColor, std::vector<glm::vec2> seats);

    // Getters
    glm::vec2 getPos() const;
    glm::vec2 getSize() const;
    glm::vec3 getColor() const;
    
    // Funzioni per gestire le sedie in modo facile
    int getSeatCount() const;
    glm::vec2 getSeatPos(int index) const; // Ti restituisce direttamente la posizione esatta della sedia nello schermo!
};
