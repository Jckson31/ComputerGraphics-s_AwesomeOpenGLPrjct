#include "TavernObjects.h"

// Costruttore base (Cucina)
TavernObject::TavernObject(glm::vec2 startPos, glm::vec2 startSize, glm::vec3 startColor) {
    pos = startPos;
    size = startSize;
    color = startColor;
}

// Costruttore per i tavoli
TavernObject::TavernObject(glm::vec2 startPos, glm::vec2 startSize, glm::vec3 startColor, std::vector<glm::vec2> tableSeats) {
    pos = startPos;
    size = startSize;
    color = startColor;
    seats = tableSeats;  //come coordinate relative al centro del tavolo, non alla posizione assoluta dello schermo
    //(ex. 40px a destra e 50px in alto)
}

glm::vec2 TavernObject::getPos() const { return pos; }
glm::vec2 TavernObject::getSize() const { return size; }
glm::vec3 TavernObject::getColor() const { return color; }
int TavernObject::getSeatCount() const { return seats.size(); }  //quanti elementi nella lista -> numero di sedie del tavolo

// Questa funzione è magica: fa il calcolo "centro del tavolo + sedia" al posto del Main!
glm::vec2 TavernObject::getSeatPos(int index) const {   //funzione per trovare le sedie
    return pos + seats[index];  // Prende la posizione del tavolo e 
    // aggiunge la posizione relativa della sedia per ottenere la posizione esatta nello schermo
}
