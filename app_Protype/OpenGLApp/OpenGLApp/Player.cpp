#include "Player.h"

Player::Player(glm::vec2 startPos) {
    pos = startPos;
    size = glm::vec2(40.0f, 40.0f);
    baseSpeed = 250.0f; // La velocità standard dell'oste

    heldItem = -1;
    score = 0;
    customersServed = 0;
    speedBoostTimer = 0.0f;

    currentFrame = 0;
    currentDir = 0;
    animTimer = 0.0f;
    isMoving = false;
}

void Player::update(float deltaTime) {
    // Gestisce il timer dello scatto in autonomia!
    if (speedBoostTimer > 0.0f) {
        speedBoostTimer -= deltaTime;
    }
}

// Spostiamo qui la logica di movimento che prima era nel Main
void Player::processMovement(GLFWwindow* window, float deltaTime, float kitchenBoundaryY) {
    float currentSpeed = baseSpeed;

    // Se c'è il bonus attivo, va il 50% più veloce!
    if (speedBoostTimer > 0.0f) {
        currentSpeed = baseSpeed * 1.5f;
    }
    float velocity = currentSpeed * deltaTime;

    // Movimento WASD
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos.y += velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos.y -= velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos.x -= velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos.x += velocity;

    // --- LOGICA ANIMAZIONE ---
    if (isMoving) {
        animTimer += deltaTime;
        if (animTimer > 0.12f) { // Cambia fotogramma ogni 0.12 secondi
            currentFrame++;
            if (currentFrame > 3) currentFrame = 0; // Se arriva al 4° frame, torna al 1° (0,1,2,3)
            animTimer = 0.0f;
        }
    }
    else {
        // Se sta fermo, usa il frame "0" (quello dritto in piedi) e azzera il timer
        currentFrame = 0;
        animTimer = 0.0f;
    }

    // Limiti dei Muri (Non farlo uscire dallo schermo)
    if (pos.x < 0.0f) pos.x = 0.0f;
    if (pos.x > 800.0f - size.x) pos.x = 800.0f - size.x;
    if (pos.y < 0.0f) pos.y = 0.0f;
    if (pos.y > 600.0f - size.y) pos.y = 600.0f - size.y;

    // Limite della Cucina (Il bancone in alto).
    // Impedisce di camminare sopra la cucina.
    if (pos.y > kitchenBoundaryY - size.y) pos.y = kitchenBoundaryY - size.y;
}

// --- FUNZIONI GETTER E SETTER ---
glm::vec2 Player::getPos() const { return pos; }
glm::vec2 Player::getSize() const { return size; }
int Player::getHeldItem() const { return heldItem; }
int Player::getScore() const { return score; }
int Player::getCustomersServed() const { return customersServed; }

int Player::getCurrentFrame() const { return currentFrame; }
int Player::getCurrentDir() const { return currentDir; }

void Player::setHeldItem(int item) { heldItem = item; }
void Player::addScore(int amount) { score += amount; }
void Player::addCustomerServed() { customersServed++; }
void Player::activateSpeedBoost(float duration) { speedBoostTimer = duration; }