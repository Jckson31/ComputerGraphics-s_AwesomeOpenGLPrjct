#include "Player.h"

Player::Player(glm::vec2 startPos) {
    pos = startPos;
    size = glm::vec2(40.0f, 60.0f);
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

glm::vec2 Player::calculateMovement(GLFWwindow* window, float deltaTime) {
    float currentSpeed = baseSpeed;

    // Applica il power-up se attivo
    if (speedBoostTimer > 0.0f) {
        currentSpeed = baseSpeed * 1.5f;
    }
    float velocity = currentSpeed * deltaTime;

    // Vettore che conterrà il nostro "desiderio" di movimento
    glm::vec2 move(0.0f, 0.0f);

    // Registriamo sia il movimento che la DIREZIONE (0=Giù, 1=Su, 2=Sinistra, 3=Destra)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { move.y += velocity; currentDir = 1; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { move.y -= velocity; currentDir = 0; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { move.x -= velocity; currentDir = 2; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { move.x += velocity; currentDir = 3; }

    isMoving = (move.x != 0.0f || move.y != 0.0f);

    // --- LOGICA ANIMAZIONE ---
    if (isMoving) {
        animTimer += deltaTime;
        if (animTimer > 0.15f) { // Puoi cambiare 0.15f per far muovere i piedi più o meno velocemente!
            currentFrame++;
            // Ora i frame sono solo due: 0 e 1!
            if (currentFrame > 1) currentFrame = 0;
            animTimer = 0.0f;
        }
    }
    else {
        currentFrame = 0;
        animTimer = 0.0f;
    }

    // Restituiamo lo spostamento al Main!
    return move;
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

void Player::setPos(glm::vec2 newPos) {
    pos = newPos;
}