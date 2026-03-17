#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Player {
private:
    glm::vec2 pos;
    glm::vec2 size;
    float baseSpeed;

    int heldItem;            // -1 se ha le mani vuote, 0-3 se ha un piatto
    int score;               // Il punteggio/monete
    int customersServed;     // Quanti piatti ha consegnato
    float speedBoostTimer;   // Il timer della combo velocità

    int currentFrame;
    int currentDir; // 0=Giù, 1=Su, 2=Sinistra, 3=Destra (dipende da come è disegnata l'immagine)
    float animTimer;
    bool isMoving;

public:
    // Costruttore
    Player(glm::vec2 startPos);

    // Funzioni di aggiornamento e movimento
    void update(float deltaTime);
    void processMovement(GLFWwindow* window, float deltaTime, float kitchenBoundaryY);

    // Getters (Per leggere i dati dal Main)
    glm::vec2 getPos() const;
    glm::vec2 getSize() const;
    int getHeldItem() const;
    int getScore() const;
    int getCustomersServed() const;

    int getCurrentFrame() const;
    int getCurrentDir() const;

    // Setters (Per modificare i dati dal Main)
    void setHeldItem(int item);
    void addScore(int amount);
    void addCustomerServed();
    void activateSpeedBoost(float duration);
};

