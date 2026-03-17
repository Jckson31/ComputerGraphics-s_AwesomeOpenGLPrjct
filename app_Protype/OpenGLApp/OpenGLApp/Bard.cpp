#include "Bard.h"

#include <iostream>

Bard::Bard(glm::vec2 startSize) {
    size = startSize;
    pos = glm::vec2(400.0f, -20.0f); // Fuori dallo schermo

    active = false;
    playing = false;
    walkingIn = false;
    walkingOut = false;

    timeToLeave = 0.0f;
    spawnTimer = 0.0f;
}

void Bard::update(float deltaTime) {
    // STATO 1: STA ENTRANDO
    if (walkingIn) {
        glm::vec2 stagePos = glm::vec2(400.0f, 150.0f);
        float dist = glm::distance(pos, stagePos);
        if (dist < 80.0f * deltaTime) {
            pos = stagePos;
            walkingIn = false;
            active = true;
            timeToLeave = 12.0f;
            std::cout << "\n🎵 IL BARDO E' SUL PALCO! Avvicinati e premi SPAZIO per farlo suonare! 🎵" << std::endl;
        }
        else {
            glm::vec2 dir = glm::normalize(stagePos - pos);
            pos += dir * 80.0f * deltaTime;
        }
    }
    // STATO 2: STA USCENDO
    else if (walkingOut) {
        glm::vec2 doorPos = glm::vec2(400.0f, -30.0f);
        float dist = glm::distance(pos, doorPos);
        if (dist < 80.0f * deltaTime) {
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
        timeToLeave -= deltaTime;
        if (timeToLeave <= 0.0f) {
            active = false;
            walkingOut = true;
            std::cout << "Hai ignorato il bardo, che se ne va a cercare un'altra taverna." << std::endl;
        }
    }
    // STATO 4: NON C'È (Timer per spawnare)
    else if (!active && !playing) {
        spawnTimer += deltaTime;
        if (spawnTimer > 25.0f) {
            spawnTimer = 0.0f;
            walkingIn = true;
            pos = glm::vec2(400.0f, -20.0f);
            std::cout << "Un cantastorie sta entrando nella taverna..." << std::endl;
        }
    }
}

// Quando premiamo SPAZIO, chiamiamo questa funzione.
// Se il player è vicino e il bardo stava aspettando, inizia a suonare!
bool Bard::tryInteract(glm::vec2 playerPos) {
    if (active && glm::distance(playerPos, pos) < 80.0f) {
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
    std::cout << "Il bardo ha finito il concerto e si incammina verso l'uscita." << std::endl;
}

glm::vec2 Bard::getPos() const { return pos; }
glm::vec2 Bard::getSize() const { return size; }
bool Bard::isPlaying() const { return playing; }
bool Bard::isVisible() const { return (active || playing || walkingIn || walkingOut); }