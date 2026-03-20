#pragma once
#include <glm/glm.hpp>


class Wizard
{
private:
    glm::vec2 pos;
    glm::vec2 size;
    bool active;       // In attesa sul palco
    bool walkingIn;    // Sta entrando
    bool walkingOut;   // Sta uscendo
    bool waitingForAnswer; // Sta aspettando che il giocatore risponda alla domanda

    float timeToLeave; // Quanto tempo aspetta prima di andarsene
    float spawnTimer;  // Timer per farlo apparire

public:
    // Costruttore
    Wizard(glm::vec2 startSize);

    // Funzione principale che gestisce tutti i suoi movimenti e timer
    void update(float deltaTime, bool isTavernBusy);

    // Funzione che il Main chiama quando premi SPAZIO per interagire
    bool tryInteract(glm::vec2 playerPos);
    void answerPrompt(bool accepted);


    // Getters per permettere al Main di disegnarlo
    glm::vec2 getPos() const;
    glm::vec2 getSize() const;
    bool isVisible() const; // Ci dice se dobbiamo disegnarlo o no
    bool isWaitingForAnswer() const;
    bool isEntering() const;

};




