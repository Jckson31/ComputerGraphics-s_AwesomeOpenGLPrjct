#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Player {
private:
    glm::vec2 pos;  //posizione corrente dell'oste nello schermo
    glm::vec2 size; //dimenzione dell'oste (larghezza e altezza)
    float baseSpeed;  //quanti pixel al secondo percorre l'oste camminando normalmente (senza bonus)

    int heldItem;            // -1 se ha le mani vuote, 0-3 se ha un piatto
    int score;               // Il punteggio/monete
    int customersServed;     // Quanti piatti ha consegnato
    float speedBoostTimer;   // Il timer del bonus velocità 

    int currentFrame;
    int currentDir; // 0=Giù, 1=Su, 2=Sinistra, 3=Destra (dipende da come è disegnata l'immagine)
    float animTimer;
    bool isMoving;

public:
    // Costruttore
    Player(glm::vec2 startPos);   //startPos è la posizione iniziale dell'oste, che sarà (400, 350)

    // Funzioni di aggiornamento e movimento
    void update(float deltaTime);   //deltaTime: tempo trascorso dall'ultimo frame, serve per aggiornare i timer dei bonus
    //controlla speedBoostTimer (>0) e lo decrementa di deltaTime, disattivando il bonus quando arriva a 0
    glm::vec2 calculateMovement(GLFWwindow* window, float deltaTime);

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
    void setPos(glm::vec2 newPos);
};


