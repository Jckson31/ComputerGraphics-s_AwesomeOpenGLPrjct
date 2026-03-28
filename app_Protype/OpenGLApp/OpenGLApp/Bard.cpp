#include "Bard.h"

#include <iostream>

Bard::Bard(glm::vec2 startSize) {

    size = startSize;
    pos = glm::vec2(400.0f, -20.0f); // fuo ri dallo schermo

    active = false;
    playing = false;
    walkingIn = false;
    walkingOut = false;

    timeToLeave = 0.0f;
    spawnTimer = 0.0f;

    currentFrame = 0;
    animTimer = 0.0f;
}

void Bard::update(float deltaTime, bool isTavernBusy) {
    // STATO 1: STA ENTRANDO
    if (walkingIn) {
        glm::vec2 stagePos = glm::vec2(400.0f, 110.0f);
        float dist = glm::distance(pos, stagePos);
        if (dist < 80.0f * deltaTime) {  //quando arriva, si ferma e aspetta l'interazione dell'oste
            pos = stagePos;
            walkingIn = false;
            active = true;  // aspetta che il player interagisca per suonare
            timeToLeave = 12.0f;
            std::cout << "\n E' entrato un BARDO!! Avvicinati e premi SPAZIO per farlo suonare! " << std::endl;
        }
        else {
            glm::vec2 dir = glm::normalize(stagePos - pos);   //direzione
            pos += dir * 80.0f * deltaTime;
        }
    }
    // STATO 2: STA USCENDO
    else if (walkingOut) {
        glm::vec2 doorPos = glm::vec2(400.0f, -30.0f);  //esce
        float dist = glm::distance(pos, doorPos);
        if (dist < 80.0f * deltaTime) { //quando arriva alla porta, scompare
            pos = doorPos;
            walkingOut = false; // Scompare definitivamente
        }
        else {
            glm::vec2 dir = glm::normalize(doorPos - pos);
            pos += dir * 80.0f * deltaTime;
        }
    }
    // STATO 3: IN ATTESA (Ma non sta ancora suonando)
    else if (active) {
        timeToLeave -= deltaTime;  //parte il timer di attesa
        if (timeToLeave <= 0.0f) {  // se scade, se ne va
            active = false;
            walkingOut = true;
            std::cout << "Hai IGNORATO il bardo, che se ne va a cercare un'altra taverna." << std::endl;
        }
    }
    // STATO 4: NON C'È (Timer per spawnare)
    else if (!active && !playing) {
        if (!isTavernBusy) {
            spawnTimer += deltaTime;
            if (spawnTimer > 25.0f) {
                spawnTimer = 0.0f;
                walkingIn = true;
                pos = glm::vec2(400.0f, -20.0f);
                // std::cout << "Un BARDO sta entrando nella taverna..." << std::endl;
            }
        }
    }

    // --- LOGICA ANIMAZIONE BARDO ---
    if (walkingIn || walkingOut || playing) {
        animTimer += deltaTime;
        if (animTimer > 0.15f) { // Velocità animazione
            currentFrame++;
            if (currentFrame > 1) currentFrame = 0; // I frame sono solo 0 e 1!
            animTimer = 0.0f;
        }
    }
    else {
        currentFrame = 0;
        animTimer = 0.0f;
    }
}

// Quando viene premuto SPAZIO, chiama questa funzione
// Se il giocatore è vicino e il bardo stava aspettando, inizia a suonare!
bool Bard::tryInteract(glm::vec2 playerPos) {
    if (active && glm::distance(playerPos, pos) < 80.0f) {  //sto aspettando e il giocatore è vicino
        active = false;
        playing = true;
        return true; // Ritorna 'vero' al Main così sa che deve attivare il bonus x2
    }
    return false;
}

// Quando scade il timer del bonus x2, il Main chiama questa funzione
void Bard::stopPlaying() {
    playing = false;
    walkingOut = true;
    std::cout << "Il bardo ha terminato l'esibizione e se ne va." << std::endl;
}

int Bard::getCurrentFrame() const { return currentFrame; }

int Bard::getActionState() const {
    if (playing) return 2;       // Stato 2: Suona
    if (walkingIn) return 1;     // Stato 1: Cammina in su
    return 0;                    // Stato 0: Cammina in giu (o è fermo)
}

glm::vec2 Bard::getPos() const { return pos; }
glm::vec2 Bard::getSize() const { return size; }
bool Bard::isPlaying() const { return playing; }
bool Bard::isVisible() const { return (active || playing || walkingIn || walkingOut); }  //il bardo sta facendo qua