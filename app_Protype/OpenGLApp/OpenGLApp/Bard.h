#pragma once
#include <glm/glm.hpp>

class Bard {
private:
    glm::vec2 pos;
    glm::vec2 size;
    bool active;       // In attesa sul palco
    bool playing;      // Sta suonando
    bool walkingIn;    // Sta entrando
    bool walkingOut;   // Sta uscendo

    float timeToLeave; // Quanto tempo aspetta prima di andarsene deluso
    float spawnTimer;  // Timer per farlo apparire

    int currentFrame;
    float animTimer;


public:
    // Costruttore
    Bard(glm::vec2 startSize);

    // Funzione principale che gestisce tutti i suoi movimenti e timer
    void update(float deltaTime, bool isTavernBusy);

    // Funzione che il Main chiama quando premi SPAZIO per interagire
    bool tryInteract(glm::vec2 playerPos);

    // Funzione che il Main chiama quando finisce il bonus dell'oro
    void stopPlaying();

    // Getters per permettere al Main di disegnarlo
    glm::vec2 getPos() const;
    glm::vec2 getSize() const;
    bool isVisible() const; // Ci dice se dobbiamo disegnarlo o no
    bool isPlaying() const;

    int getCurrentFrame() const;
    int getActionState() const;
};

