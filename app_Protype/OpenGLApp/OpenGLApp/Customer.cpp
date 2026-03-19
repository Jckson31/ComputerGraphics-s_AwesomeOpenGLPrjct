#include "Customer.h"

#include <glm/geometric.hpp> // Per le funzioni distance e normalize

Customer::Customer(int tIdx, int sIdx, glm::vec2 startPos, glm::vec2 target, int food, float startPatience) {
    table = tIdx;
    seat = sIdx;
    pos = startPos;
    targetPos = target;  //le coordinate esatte della sedia in cui si andrà a sedere il cliente
    desiredFood = food;
    patience = startPatience;

    isWalking = true;
    walkState = 0;
    // Logica avanzata a 4 corridoi!
    if (targetPos.x < 100.0f) {
        safeX = 50.0f;  // Se la sedia è tutta a sinistra, usa il corridoio rasente al muro sinistro
    }
    else if (targetPos.x > 700.0f) {
        safeX = 750.0f; // Se la sedia è tutta a destra, usa il corridoio rasente al muro destro
    }
    else if (targetPos.x < 400.0f) {
        safeX = 270.0f; // Sedia interna sinistra -> corridoio centrale-sinistro
    }
    else {
        safeX = 530.0f; // Sedia interna destra -> corridoio centrale-destro
    }                                                  //"Se la sedia di destinazione si trova a sinistra dello schermo (< 400),
                                                       // usa il corridoio a X=270. Altrimenti, usa il corridoio a X=530"
                                                       //Cosi non passano attraverso il tavolo ma fanno percorsi sicuri
    waiting = true;
    isAngry = false;
}

void Customer::update(float deltaTime) {
    if (isWalking) {
        float speed = 120.0f * deltaTime;
        glm::vec2 nextTarget;

        if (walkState == 0) nextTarget = glm::vec2(400.0f, 30.0f);  // Vai alla porta della taverna  
        else if (walkState == 1) nextTarget = glm::vec2(safeX, 30.0f); //vai verso il corridoio sicuro (stesso Y della porta)
        else if (walkState == 2) nextTarget = glm::vec2(safeX, targetPos.y); //sali lungo il corridoio vers la Y giusta
        else nextTarget = targetPos;  //arriva alla sedia 

        float distance = glm::distance(pos, nextTarget);  //quanti pixel mancano al cliente per arrivare alla tappa
        if (distance <= speed) {  //se la distanza è minore della velocità , il cliente è arrivato a destinazione
            pos = nextTarget;
            walkState++;
            if (walkState > 3) isWalking = false;  //se è arrivato alla sedia, smette di camminare
        }
        else {
            glm::vec2 direction = glm::normalize(nextTarget - pos);  //calcola la direzione 
            pos += direction * speed;
        }
    }
    else {  // è fermo perche ha raggiunto la sedia, quindi inizia a perdere pazienza
        patience -= deltaTime;
        if (patience <= 0.0f) {
            isAngry = true;
        }
    }
}

glm::vec2 Customer::getPos() const { return pos; }
float Customer::getPatience() const { return patience; }
bool Customer::getIsWalking() const { return isWalking; }