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

    currentFrame = 0;
    animTimer = 0.0f;
    currentDir = 0;

    float r = (rand() % 100) / 100.0f;
    float g = (rand() % 100) / 100.0f;
    float b = (rand() % 100) / 100.0f;
    clothesColor = glm::vec3(r, g, b);

    // Colore casuale dei capelli!
    float hr = (rand() % 100) / 100.0f;
    float hg = (rand() % 100) / 100.0f;
    float hb = (rand() % 100) / 100.0f;
    hairColor = glm::vec3(hr, hg, hb);

    isEating = false;
    eatingTimer = 0.0f;
    doneEating = false;

    isLeaving = false;
    hasLeft = false;
}

void Customer::update(float deltaTime, bool isBardPlaying) {
    if (isEating) {
        eatingTimer += deltaTime;
        if (eatingTimer > 3.0f) { // Rimane seduto a mangiare per 3 secondi!
            isEating = false;
            doneEating = true;
            leave(); // Finito il tempo, si alza e se ne va
        }
    }
    else if (isWalking) {
        float speed = 120.0f * deltaTime;
        glm::vec2 nextTarget;

        if (walkState == 0) nextTarget = glm::vec2(400.0f, 30.0f);  // Vai alla porta della taverna  
        else if (walkState == 1) nextTarget = glm::vec2(safeX, 30.0f); //vai verso il corridoio sicuro (stesso Y della porta)
        else if (walkState == 2) nextTarget = glm::vec2(safeX, targetPos.y); //sali lungo il corridoio vers la Y giusta
        else if (walkState == 3) nextTarget = targetPos;  //arriva alla sedia 

        else if (walkState == 4) nextTarget = glm::vec2(safeX, targetPos.y); // Torna al corridoio sicuro
        else if (walkState == 5) nextTarget = glm::vec2(safeX, 30.0f);       // Scendi verso la porta
        else if (walkState == 6) nextTarget = glm::vec2(400.0f, -20.0f);     // Esci dallo schermo

        float distance = glm::distance(pos, nextTarget);  //quanti pixel mancano al cliente per arrivare alla tappa
        if (distance <= speed) {  //se la distanza è minore della velocità , il cliente è arrivato a destinazione
            pos = nextTarget;
            walkState++;

            if (walkState == 4 && !isLeaving)
                isWalking = false;  //"si alza" e va verso il corridoio sicuro
            else if (walkState > 6) {  // è uscito dallo schermo, possiamo eliminarlo
                isWalking = false;
                hasLeft = true; // Ora il Main può cancellarlo!
            }
        }
        else {
            glm::vec2 direction = glm::normalize(nextTarget - pos);  //calcola la direzione 
            pos += direction * speed;
        }

        // --- LOGICA DIREZIONE ---
        float dx = nextTarget.x - pos.x;
        float dy = nextTarget.y - pos.y;

        if (std::abs(dx) > std::abs(dy)) {
            if (dx < 0) currentDir = 2; // Sinistra
            else currentDir = 3;        // Destra
        }
        else {
            if (dy > 0) currentDir = 1; // Su
            else currentDir = 0;        // Giù
        }

        // --- LOGICA ANIMAZIONE ---
        animTimer += deltaTime;
        if (animTimer > 0.15f) {
            currentFrame++;
            if (currentFrame > 1) currentFrame = 0;
            animTimer = 0.0f;
        }
    }
    else {  // è fermo perche ha raggiunto la sedia, quindi inizia a perdere pazienza
        if (isBardPlaying) {
            // Se il bardo suona, i clienti si distraggono e perdono pazienza più lentamente (metà velocità!)
            patience -= (deltaTime * 0.5f);
        }
        else {
            // Altrimenti, perdono pazienza normalmente
            patience -= deltaTime;
        }

        if (patience <= 0.0f) {
            isAngry = true;
        }
    }
}

glm::vec2 Customer::getPos() const { return pos; }
float Customer::getPatience() const { return patience; }
bool Customer::getIsWalking() const { return isWalking; }
int Customer::getCurrentFrame() const { return currentFrame; }
int Customer::getCurrentDir() const { return currentDir; }
glm::vec3 Customer::getClothesColor() const { return clothesColor; }
glm::vec3 Customer::getHairColor() const { return hairColor; }

void Customer::servedFood() {
    isEating = true;
    waiting = false;     // Non aspetta più l'ordinazione
    eatingTimer = 0.0f;  // Fa partire il timer del pasto a zero
}

bool Customer::getIsEating() const { return isEating; }

void Customer::leave() {
    isLeaving = true;
    isWalking = true;
    walkState = 4;   // Iniziamo la sequenza di uscita dal corridoio
    waiting = false; // Smette di aspettare il cibo (evita che tu possa servirlo mentre scappa)
}

bool Customer::getIsLeaving() const { return isLeaving; }
bool Customer::getHasLeft() const { return hasLeft; }

