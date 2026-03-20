#pragma once
#include <glm/glm.hpp>

class Customer {
private:
    glm::vec2 pos;
    glm::vec2 targetPos;
    bool isWalking;
    int walkState;
    float safeX;
    float patience;

    int currentFrame;
    float animTimer;
    int currentDir; // 0=Giu, 1=Su, 2=Sinistra, 3=Destra
    glm::vec3 clothesColor; // Il colore casuale!
    glm::vec3 hairColor;

public:
    int table;
    int seat;
    int desiredFood;
    bool waiting;
    bool isAngry;


    // Costruttore
    Customer(int tIdx, int sIdx, glm::vec2 startPos, glm::vec2 target, int food, float startPatience);

    // Funzione principale
    void update(float deltaTime, bool isBardPlaying);

    // Funzioni per leggere i dati privati dal Main
    glm::vec2 getPos() const;
    float getPatience() const;
    bool getIsWalking() const;

    int getCurrentFrame() const;
    int getCurrentDir() const;
    glm::vec3 getClothesColor() const;
    glm::vec3 getHairColor() const;
};




/*
// --- CLIENTI ---
//struct Customer {
//    glm::vec2 pos;           // posizione attuale del cliente nello schermo
//    glm::vec2 targetPos;     // sedia da raggiungere
 //   bool isWalking;          // Sta camminando verso il tavolo?
//
//    int walkState;       // NUOVO: 0=Entra, 1=Vai al corridoio, 2=Sali, 3=Vai alla sedia
//    float safeX;         // NUOVO: La coordinata X del corridoio sicuro da percorrere

//    int table;      // A quale tavolo è seduto (0, 1, 2, 3 o 4)
 //   int seat;       // In quale sedia è seduto (0 o 1, o 0-3 per il tavolo centrale)
//    int desiredFood;     // Indice della stazione che desidera (0, 1, 2 o 3)
//    float patience;      // Timer prima che se ne vada arrabbiato (lo useremo dopo)
//    bool waiting; // true se aspetta di mangiare
//};

*/