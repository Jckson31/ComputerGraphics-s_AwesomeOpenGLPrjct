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

    bool isEating;        // Vero se il cliente sta mangiando
    float eatingTimer;    // Conta da quanto tempo sta mangiando

    bool isLeaving;       // Vero se il cliente se ne sta andando
    bool hasLeft;  // Vero quando il cliente è fisicamente fuori dallo schermo

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

    void servedFood();
    bool getIsEating() const;

    void leave();
    bool getIsLeaving() const;
    bool getHasLeft() const;
};