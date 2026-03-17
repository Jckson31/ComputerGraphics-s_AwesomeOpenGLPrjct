#include "Customer.h"

#include <glm/geometric.hpp> // Per le funzioni distance e normalize

Customer::Customer(int tIdx, int sIdx, glm::vec2 startPos, glm::vec2 target, int food, float startPatience) {
    table = tIdx;
    seat = sIdx;
    pos = startPos;
    targetPos = target;
    desiredFood = food;
    patience = startPatience;

    isWalking = true;
    walkState = 0;
    safeX = (targetPos.x < 400.0f) ? 270.0f : 530.0f;
    waiting = true;
    isAngry = false;
}

void Customer::update(float deltaTime) {
    if (isWalking) {
        float speed = 120.0f * deltaTime;
        glm::vec2 nextTarget;

        if (walkState == 0) nextTarget = glm::vec2(400.0f, 30.0f);
        else if (walkState == 1) nextTarget = glm::vec2(safeX, 30.0f);
        else if (walkState == 2) nextTarget = glm::vec2(safeX, targetPos.y);
        else nextTarget = targetPos;

        float distance = glm::distance(pos, nextTarget);
        if (distance <= speed) {
            pos = nextTarget;
            walkState++;
            if (walkState > 3) isWalking = false;
        }
        else {
            glm::vec2 direction = glm::normalize(nextTarget - pos);
            pos += direction * speed;
        }
    }
    else {
        patience -= deltaTime;
        if (patience <= 0.0f) {
            isAngry = true;
        }
    }
}

glm::vec2 Customer::getPos() const { return pos; }
float Customer::getPatience() const { return patience; }
bool Customer::getIsWalking() const { return isWalking; }