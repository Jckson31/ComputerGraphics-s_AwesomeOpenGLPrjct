#pragma once
#include <vector>
#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_s.h"

// Le tue classi
#include "Player.h"
#include "Customer.h"
#include "TavernObjects.h"
#include "FoodStations.h"
#include "Dirt.h"
#include "Bard.h"
#include "Wizard.h"
#include "Model.h"

// Stati del gioco (preparazione per il futuro Menu!)
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_OVER
};

class Game {
public:
    // --- STATO DEL GIOCO E FINESTRA ---
    GameState State;
    unsigned int Width, Height;
    bool isGameOver;

    // --- VARIABILI DI PROGRESSIONE E PUNTEGGIO ---
    int score;
    int currentLevel;
    int customersServed;
    int angryCustomers;
    const int MAX_ANGRY_CUSTOMERS = 10;
    const int MAX_DIRT = 10;

    float currentSpawnRate;
    float currentMaxPatience;

    // --- TIMER GLOBALI ---
    float randomDirtTimer;
    float spawnTimer;
    float speedBoostTimer;
    float goldBoostTimer;

    // --- GESTIONE INPUT (Evita che premendo un tasto faccia 100 azioni) ---
    bool spacebarPressed;
    bool cWasPressed;
    bool eWasPressed;

    // --- PUNTATORI AGLI OGGETTI UNICI ---
    Player* player;
    Bard* bard;
    Wizard* wizard;
    TavernObject* counter;
    TavernObject* trashCan;

    // --- LISTE DI OGGETTI MULTIPLI ---
    std::vector<Customer> customers;
    std::vector<TavernObject> tables;
    std::vector<FoodStation> foodStations;
    std::vector<Dirt> dirts;

    // --- GESTIONE GRAFICA E RENDER ---
    unsigned int cubeVAO;

    // Texture singole
    unsigned int texTrash;
    unsigned int texCustSeated;
    unsigned int texWizUp, texWizDown;
    unsigned int texBardUp, texBardDown, texBardPlay;

    // Array di texture
    unsigned int foodTextures[4];
    unsigned int playerTextures[4];
    unsigned int custTextures[4];

    // --- METODI ---
    // Costruttore e distruttore
    Game(unsigned int width, unsigned int height);
    ~Game();

    // Funzioni principali del ciclo di gioco
    void Init();
    void ProcessInput(GLFWwindow* window, float dt);
    void Update(float dt);

    // Funzioni di utilità
    bool CheckCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2);
    void PrintDashboard();

    void Render(class Shader& spriteShader, class Shader& modelShader, class Model& tableModel, class Model& counterModel);
    void drawTavernModel(class Shader& shader, class Model& model, float screenX, float screenY, float scaleX, float scaleY, float scaleZ, float rotX, float rotY = 0.0f);
};
