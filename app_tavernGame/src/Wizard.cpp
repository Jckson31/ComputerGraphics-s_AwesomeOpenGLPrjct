#include "Wizard.h"
#include <iostream>

Wizard::Wizard(glm::vec2 startSize) {
	size = startSize;
	pos = glm::vec2(400.0f, -20.0f); // fuori dallo schermo

	active = false;
	walkingIn = false;
	walkingOut = false;
	waitingForAnswer = false;

	timeToLeave = 0.0f;
	spawnTimer = 0.0f;
}


void Wizard::update(float deltaTime, bool isTavernBusy) {
	//STATO 1
	if (walkingIn) {
		// STA ENTRANDO
		glm::vec2 stagePos = glm::vec2(400.0f, 150.0f);
		float dist = glm::distance(pos, stagePos);
		if (dist < 80.0f * deltaTime) { // arriva, si ferma e aspetta
			pos = stagePos;
			walkingIn = false;
			active = true;
			timeToLeave = 12.0f;
			std::cout << "\n E' entrato un misterioso STREGONE!! Se gli dai 15 monete, ti aiuterà a PULIRE  il locale! Avvicinati e premi SPAZIO per interagire! " << std::endl;
		}
		else {
			glm::vec2 dir = glm::normalize(stagePos - pos);  //direzione
			pos += dir * 80.0f * deltaTime;
		}


	}
	//STATO 2
	//STA USCENDO
	else if (walkingOut) {
		glm::vec2 stagePos = glm::vec2(400.0f, -30.0f);  //esce	
		float dist = glm::distance(pos, stagePos);
		if (dist < 80.0f * deltaTime) { //quando arriva alla porta, scompare
			pos = stagePos;
			walkingOut = false; // Scompare definitivamente
		}
		else {
			glm::vec2 dir = glm::normalize(stagePos - pos);
			pos += dir * 80.0f * deltaTime;
		}
	}
	// IN ATTESA DI INTERAZIONE
	else if (active) {
		timeToLeave -= deltaTime;  //parte il timer di attesa
		if (timeToLeave <= 0.0f) {  // se scade, se ne va
			active = false;
			walkingOut = true;
			std::cout << "Hai IGNORATO lo stregone, che se ne va a cercare un'altra taverna." << std::endl;
		}


	}
	// TIMER PER SPAWNARE
	else if (!active && !waitingForAnswer) {
		if (!isTavernBusy) {
			spawnTimer += deltaTime;
			if (spawnTimer > 20.0f) { // Ogni 20 secondi
				spawnTimer = 0.0f; // Resetta il timer
				walkingIn = true; // Fa entrare lo stregone
			}
		}
	}
}


bool Wizard::tryInteract(glm::vec2 playerPos) {
	if (active && !waitingForAnswer && glm::distance(playerPos, pos) < 80.0f) {
		waitingForAnswer = false;
		return true;
	}
	return false;
}

void Wizard::answerPrompt(bool accepted) {
	waitingForAnswer = false;
	active = false;
	walkingOut = true;  // se ne va in ogni caso dopo aver risposto

	if (accepted) {
		std::cout << "WOOSH! Il mago fa un INCANTESIMO e la taverna è PULITA!!" << std::endl;
	}
	else {
		std::cout << "Il mago fa SPALLUCCE e si avvia verso l'uscità." << std::endl;
	}
}

glm::vec2 Wizard::getPos() const { return pos; }
glm::vec2 Wizard::getSize() const { return size; }
bool Wizard::isVisible() const { return (active || walkingIn || walkingOut || waitingForAnswer); }
bool Wizard::isWaitingForAnswer() const { return waitingForAnswer; }

#include "Wizard.h"
