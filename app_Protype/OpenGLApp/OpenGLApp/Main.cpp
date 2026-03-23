/*
================================================================================
                    GIOCO DELLA TAVERNA - VERSIONE ORGANIZZATA
================================================================================
Versione del codice originale, solo riorganizzato e pulito.
Nessun cambio a gameplay, texture, o comandi - tutto rimane uguale!
================================================================================
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"

#include <iostream>
#include <vector>
#include <cstdlib>

#include "Customer.h"
#include "Bard.h"
#include "Player.h"
#include "TavernObjects.h"
#include "FoodStations.h"
#include "Dirt.h"
#include "Wizard.h"

// ═══════════════════════════════════════════════════════════════════════════
//                         CONFIGURAZIONE SCHERMO
// ═══════════════════════════════════════════════════════════════════════════
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ═══════════════════════════════════════════════════════════════════════════
//                         VARIABILI DI TIMING
// ═══════════════════════════════════════════════════════════════════════════
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ═══════════════════════════════════════════════════════════════════════════
//                         STATO DI GIOCO
// ═══════════════════════════════════════════════════════════════════════════
int score = 0;
int currentLevel = 1;
int customersServed = 0;
int angryCustomers = 0;

// Difficoltà dinamica
float currentSpawnRate = 10.0f;
float currentMaxPatience = 50.0f;

// Bonus
float goldBoostTimer = 0.0f;

// Gestione input
bool spacebarPressed = false;
bool cWasPressed = false;
bool eWasPressed = false;

// ═══════════════════════════════════════════════════════════════════════════
//                      LIMITI E PARAMETRI GIOCO
// ═══════════════════════════════════════════════════════════════════════════
const int MAX_DIRT = 10;
float randomDirtTimer = 0.0f;

// ═══════════════════════════════════════════════════════════════════════════
//                      OGGETTI DI GIOCO
// ═══════════════════════════════════════════════════════════════════════════
Player player(glm::vec2(400.0f, 350.0f));
TavernObject counter(glm::vec2(400.0f, 550.0f), glm::vec2(600.0f, 100.0f), glm::vec3(0.6f, 0.3f, 0.1f));
std::vector<TavernObject> tables;
std::vector<FoodStation> foodStations;
std::vector<Customer> customers;
std::vector<Dirt> dirts;
float spawnTimer = 0.0f;

Bard bard(glm::vec2(40.0f, 40.0f));
Wizard wizard(glm::vec2(40.0f, 40.0f));

// ═══════════════════════════════════════════════════════════════════════════
//                      FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// ═══════════════════════════════════════════════════════════════════════════
//                      FUNZIONI UTILITY
// ═══════════════════════════════════════════════════════════════════════════

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true); // Flippa le texture per visualizzarle correttamente

    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture non trovata nel percorso: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

bool checkCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2)
{
    bool collisionX = (pos1.x + size1.x / 2.0f) >= (pos2.x - size2.x / 2.0f) &&
        (pos1.x - size1.x / 2.0f) <= (pos2.x + size2.x / 2.0f);

    bool collisionY = (pos1.y + size1.y / 2.0f) >= (pos2.y - size2.y / 2.0f) &&
        (pos1.y - size1.y / 2.0f) <= (pos2.y + size2.y / 2.0f);

    return collisionX && collisionY;
}

void checkTavernDirt() {
    int currentDirt = (int)dirts.size();

    std::cout << "\nLIVELLO SPORCIZIA: [";
    for (int i = 0; i < MAX_DIRT; i++) {
        if (i < currentDirt) std::cout << "#";
        else std::cout << "-";
    }
    std::cout << "] " << currentDirt << "/" << MAX_DIRT << "\n" << std::endl;

    if (currentDirt >= MAX_DIRT) {
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cout << "               GAME OVER                " << std::endl;
        std::cout << " L'ispettore sanitario ha chiuso il tuo " << std::endl;
        std::cout << "  locale perche' era un porcile!        " << std::endl;
        std::cout << " Punteggio Finale: " << score << " Monete" << std::endl;
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

        score = 0;
        dirts.clear();
    }
}

void initializeTables()
{
    tables.clear();

    std::vector<glm::vec2> sideSeats = {
        glm::vec2(0.0f, 45.0f),
        glm::vec2(0.0f, -45.0f),
        glm::vec2(-75.0f, 0.0f),
        glm::vec2(75.0f, 0.0f)
    };

    std::vector<glm::vec2> centerSeats = {
        glm::vec2(-40.0f, 55.0f),
        glm::vec2(40.0f, 55.0f),
        glm::vec2(-40.0f, -55.0f),
        glm::vec2(40.0f, -55.0f),
        glm::vec2(-95.0f, 0.0f),
        glm::vec2(95.0f, 0.0f)
    };

    tables.push_back(TavernObject(glm::vec2(150.0f, 400.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(150.0f, 100.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(650.0f, 400.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(650.0f, 100.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(400.0f, 250.0f), glm::vec2(160.0f, 80.0f), glm::vec3(0.4f, 0.2f, 0.1f), centerSeats));
}

void initializeFoodStations()
{
    foodStations.clear();
    foodStations.push_back(FoodStation(glm::vec2(150.0f, 550.0f), glm::vec2(40.0f, 40.0f), 0));
    foodStations.push_back(FoodStation(glm::vec2(315.0f, 550.0f), glm::vec2(40.0f, 40.0f), 1));
    foodStations.push_back(FoodStation(glm::vec2(485.0f, 550.0f), glm::vec2(40.0f, 40.0f), 2));
    foodStations.push_back(FoodStation(glm::vec2(650.0f, 550.0f), glm::vec2(40.0f, 40.0f), 3));
}

// ═══════════════════════════════════════════════════════════════════════════
//                         MAIN
// ═══════════════════════════════════════════════════════════════════════════
int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "App", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shader program
    Shader ourShader("shader.vs", "shader.fs");

    // set up vertex data
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load textures
    unsigned int texFloor = loadTexture("resources/textures/wood.jpeg");
    unsigned int texPlayer = loadTexture("resources/textures/awesomeface.png");
    unsigned int texBeer = loadTexture("resources/textures/beer.png");
    unsigned int texMeat = loadTexture("resources/textures/meat.png");
    unsigned int texBread = loadTexture("resources/textures/bread.png");
    unsigned int texSoup = loadTexture("resources/textures/soup.png");
    unsigned int texCustomer = loadTexture("resources/textures/awesomeface.png");

    unsigned int foodTextures[4];
    foodTextures[0] = texBeer;
    foodTextures[1] = texMeat;
    foodTextures[2] = texSoup;
    foodTextures[3] = texBread;

    ourShader.use();
    ourShader.setInt("texture1", 0);

    // initialize game
    initializeTables();
    initializeFoodStations();

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // ─────────────────────────────────────────────────────────
        //                    GAME LOGIC
        // ─────────────────────────────────────────────────────────

        // Sporco casuale
        randomDirtTimer += deltaTime;
        if (randomDirtTimer > 8.0f) {
            randomDirtTimer = 0.0f;
            float randomX = 50.0f + (rand() % 700);
            float randomY = 50.0f + (rand() % 400);
            dirts.push_back(Dirt(glm::vec2(randomX, randomY)));
            std::cout << "E' apparsa una macchia sul pavimento..." << std::endl;
            checkTavernDirt();
        }

        // Aggiorna giocatore
        player.update(deltaTime);

        // Aggiorna bonus Bardo
        if (goldBoostTimer > 0.0f) {
            goldBoostTimer -= deltaTime;
            if (goldBoostTimer <= 0.0f) {
                bard.stopPlaying();
                std::cout << "Il bardo ha TERMINATO l'esibizione e se ne va." << std::endl;
            }
        }

        // Aggiorna Bardo e Mago
        bard.update(deltaTime, wizard.isVisible());
        wizard.update(deltaTime, bard.isVisible());

        // Spawn clienti
        spawnTimer += deltaTime;
        if (spawnTimer > currentSpawnRate) {
            spawnTimer = 0.0f;

            std::vector<std::pair<int, int>> freeSeats;

            for (int i = 0; i < (int)tables.size(); i++) {
                for (int j = 0; j < tables[i].getSeatCount(); j++) {
                    bool isFree = true;
                    for (int k = 0; k < (int)customers.size(); k++) {
                        if (customers[k].table == i && customers[k].seat == j) {
                            isFree = false;
                            break;
                        }
                    }
                    if (isFree) {
                        freeSeats.push_back(std::pair<int, int>(i, j));
                    }
                }
            }

            if (freeSeats.size() > 0) {
                int randomIndex = rand() % (int)freeSeats.size();
                int chosenTable = freeSeats[randomIndex].first;
                int chosenSeat = freeSeats[randomIndex].second;

                glm::vec2 startPos(400.0f, -20.0f);
                glm::vec2 target = tables[chosenTable].getSeatPos(chosenSeat);
                int randomFood = rand() % 4;

                Customer newCust(chosenTable, chosenSeat, startPos, target, randomFood, currentMaxPatience);
                customers.push_back(newCust);
            }
        }

        // Aggiorna clienti
        for (int i = 0; i < (int)customers.size(); ) {
            customers[i].update(deltaTime);

            if (customers[i].isAngry) {
                angryCustomers++;
                std::cout << "Un cliente se n'e' andato arrabbiato! -5 Monete!" << std::endl;
                score -= 5;

                if (rand() % 100 < 40) {
                    dirts.push_back(Dirt(customers[i].getPos()));
                    checkTavernDirt();
                }
                customers.erase(customers.begin() + i);
            }
            else {
                i++;
            }
        }

        // ─────────────────────────────────────────────────────────
        //                    RENDERING
        // ─────────────────────────────────────────────────────────
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texFloor);

        ourShader.use();

        glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
        ourShader.setMat4("projection", projection);

        glm::mat4 view = glm::mat4(1.0f);
        ourShader.setMat4("view", view);

        glBindVertexArray(cubeVAO);

        // Disegna Bancone
        ourShader.setVec3("noteColor", counter.getColor());
        glm::mat4 modelKitchen = glm::mat4(1.0f);
        modelKitchen = glm::translate(modelKitchen, glm::vec3(counter.getPos().x, counter.getPos().y, 0.0f));
        modelKitchen = glm::scale(modelKitchen, glm::vec3(counter.getSize().x, counter.getSize().y, 1.0f));
        ourShader.setMat4("model", modelKitchen);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Disegna Tavoli
        for (int i = 0; i < (int)tables.size(); i++) {
            ourShader.setVec3("noteColor", tables[i].getColor());
            glm::mat4 modelTable = glm::mat4(1.0f);
            modelTable = glm::translate(modelTable, glm::vec3(tables[i].getPos().x, tables[i].getPos().y, 0.0f));
            modelTable = glm::scale(modelTable, glm::vec3(tables[i].getSize().x, tables[i].getSize().y, 1.0f));
            ourShader.setMat4("model", modelTable);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Disegna Giocatore
        ourShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texPlayer);
        ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

        glm::mat4 modelPlayer = glm::mat4(1.0f);
        modelPlayer = glm::translate(modelPlayer, glm::vec3(player.getPos().x, player.getPos().y, 0.15f));
        modelPlayer = glm::scale(modelPlayer, glm::vec3(player.getSize().x, player.getSize().y, 1.0f));
        ourShader.setMat4("model", modelPlayer);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Disegna cibo in mano
        int currentItem = player.getHeldItem();
        if (currentItem != -1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, foodTextures[currentItem]);

            ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));
            glm::mat4 modelHeld = glm::mat4(1.0f);
            modelHeld = glm::translate(modelHeld, glm::vec3(player.getPos().x, player.getPos().y + 40.0f, 0.3f));
            modelHeld = glm::scale(modelHeld, glm::vec3(20.0f, 20.0f, 1.0f));
            ourShader.setMat4("model", modelHeld);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Disegna Stazioni
        for (int i = 0; i < (int)foodStations.size(); i++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, foodTextures[i]);

            ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

            glm::mat4 modelStation = glm::mat4(1.0f);
            modelStation = glm::translate(modelStation, glm::vec3(foodStations[i].getPos().x, foodStations[i].getPos().y, 0.1f));
            modelStation = glm::scale(modelStation, glm::vec3(foodStations[i].getSize().x, foodStations[i].getSize().y, 1.0f));
            ourShader.setMat4("model", modelStation);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Disegna Sporco
        for (int i = 0; i < (int)dirts.size(); i++) {
            ourShader.setVec3("noteColor", glm::vec3(0.3f, 0.4f, 0.1f));
            glm::mat4 modelDirt = glm::mat4(1.0f);
            modelDirt = glm::translate(modelDirt, glm::vec3(dirts[i].getPos().x, dirts[i].getPos().y, 0.05f));
            modelDirt = glm::scale(modelDirt, glm::vec3(dirts[i].getSize().x, dirts[i].getSize().y, 1.0f));
            ourShader.setMat4("model", modelDirt);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Disegna Bardo
        if (bard.isVisible()) {
            if (bard.isPlaying()) {
                ourShader.setVec3("noteColor", glm::vec3(0.9f, 0.4f, 0.8f));
            }
            else {
                ourShader.setVec3("noteColor", glm::vec3(0.6f, 0.2f, 0.8f));
            }

            glm::mat4 modelBard = glm::mat4(1.0f);
            modelBard = glm::translate(modelBard, glm::vec3(bard.getPos().x, bard.getPos().y, 0.12f));
            modelBard = glm::scale(modelBard, glm::vec3(bard.getSize().x, bard.getSize().y, 1.0f));
            ourShader.setMat4("model", modelBard);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Disegna Mago
        if (wizard.isVisible()) {
            ourShader.setVec3("noteColor", glm::vec3(0.2f, 0.7f, 1.0f));

            glm::mat4 modelWizard = glm::mat4(1.0f);
            modelWizard = glm::translate(modelWizard, glm::vec3(wizard.getPos().x, wizard.getPos().y, 0.12f));
            modelWizard = glm::scale(modelWizard, glm::vec3(wizard.getSize().x, wizard.getSize().y, 1.0f));
            ourShader.setMat4("model", modelWizard);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Disegna Clienti
        for (int i = 0; i < (int)customers.size(); i++) {
            glm::vec2 custPos = customers[i].getPos();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texCustomer);

            glm::vec3 custColor = glm::vec3(1.0f, 1.0f, 1.0f);

            if (!customers[i].getIsWalking()) {
                if (customers[i].getPatience() > 20.0f) {
                    custColor = glm::vec3(0.2f, 0.3f, 0.8f);
                }
                else if (customers[i].getPatience() > 10.0f) {
                    custColor = glm::vec3(0.9f, 0.5f, 0.1f);
                }
                else {
                    custColor = glm::vec3(0.8f, 0.1f, 0.1f);
                }
            }

            ourShader.setVec3("noteColor", custColor);
            glm::mat4 modelCust = glm::mat4(1.0f);
            modelCust = glm::translate(modelCust, glm::vec3(custPos.x, custPos.y, 0.11f));
            modelCust = glm::scale(modelCust, glm::vec3(40.0f, 40.0f, 1.0f));
            ourShader.setMat4("model", modelCust);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            if (!customers[i].getIsWalking()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, foodTextures[customers[i].desiredFood]);

                ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));
                glm::mat4 modelOrder = glm::mat4(1.0f);

                modelOrder = glm::translate(modelOrder, glm::vec3(custPos.x, custPos.y + 35.0f, 0.12f));
                modelOrder = glm::scale(modelOrder, glm::vec3(25.0f, 25.0f, 1.0f));
                ourShader.setMat4("model", modelOrder);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);

    glfwTerminate();
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
//                      CALLBACK E INPUT
// ═══════════════════════════════════════════════════════════════════════════

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movimento
    glm::vec2 moveDir = player.calculateMovement(window, deltaTime);
    glm::vec2 playerPos = player.getPos();
    glm::vec2 playerSize = player.getSize();

    glm::vec2 nextPosX = playerPos;
    nextPosX.x += moveDir.x;

    glm::vec2 nextPosY = playerPos;
    nextPosY.y += moveDir.y;

    float halfWidth = playerSize.x / 2.0f;
    float halfHeight = playerSize.y / 2.0f;

    if (nextPosX.x - halfWidth < 0.0f) nextPosX.x = halfWidth;
    if (nextPosX.x + halfWidth > SCR_WIDTH) nextPosX.x = SCR_WIDTH - halfWidth;
    if (nextPosY.y - halfHeight < 0.0f) nextPosY.y = halfHeight;
    if (nextPosY.y + halfHeight > SCR_HEIGHT) nextPosY.y = SCR_HEIGHT - halfHeight;

    bool colX = false;
    if (checkCollision(nextPosX, playerSize, counter.getPos(), counter.getSize())) colX = true;
    for (int i = 0; i < (int)tables.size(); i++) {
        if (checkCollision(nextPosX, playerSize, tables[i].getPos(), tables[i].getSize())) colX = true;
    }
    if (!colX) playerPos.x = nextPosX.x;

    bool colY = false;
    if (checkCollision(nextPosY, playerSize, counter.getPos(), counter.getSize())) colY = true;
    for (int i = 0; i < (int)tables.size(); i++) {
        if (checkCollision(nextPosY, playerSize, tables[i].getPos(), tables[i].getSize())) colY = true;
    }
    if (!colY) playerPos.y = nextPosY.y;

    player.setPos(playerPos);

    // Spazio
    bool spaceIsPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    bool cIsPressed = (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);

    if (spaceIsPressed && !spacebarPressed) {
        bool interactedWithNPC = false;

        if (bard.tryInteract(player.getPos())) {
            goldBoostTimer = 15.0f;
            interactedWithNPC = true;
            std::cout << "\n[BARDO] BONUS ATTIVO! Suona per guadagni doppi!\n" << std::endl;
        }

        if (!interactedWithNPC && wizard.tryInteract(player.getPos())) {
            interactedWithNPC = true;
            std::cout << "\n[MAGO] Offro: 15 monete per spazzare tutto lo sporco!" << std::endl;
            std::cout << ">>> Premi [Y] per pagare, [N] per rifiutare. <<<\n" << std::endl;
        }

        if (!interactedWithNPC) {
            if (player.getHeldItem() == -1) {
                for (int i = 0; i < (int)foodStations.size(); i++) {
                    float distanza = glm::distance(player.getPos(), foodStations[i].getPos());
                    if (distanza < 120.0f) {
                        player.setHeldItem(i);
                        std::cout << "Raccolto piatto: " << i << std::endl;
                        break;
                    }
                }
            }
            else {
                bool delivered = false;
                for (int i = 0; i < (int)customers.size(); i++) {
                    int tId = customers[i].table;
                    int sId = customers[i].seat;

                    glm::vec2 custPos = tables[tId].getSeatPos(sId);

                    float distanza = glm::distance(player.getPos(), custPos);

                    if (distanza < 120.0f && customers[i].waiting && customers[i].desiredFood == player.getHeldItem()) {

                        int baseReward = (goldBoostTimer > 0.0f) ? 20 : 10;
                        float pazienza = customers[i].getPatience();

                        if (pazienza <= 10.0f) {
                            baseReward = baseReward / 2;
                        }
                        else if (pazienza <= 20.0f) {
                            baseReward = (int)(baseReward * 0.75f);
                        }

                        int dirtPenalty = (int)dirts.size();
                        int finalReward = baseReward - dirtPenalty;

                        if (finalReward < 1) finalReward = 1;

                        score += finalReward;
                        customersServed++;

                        std::cout << "Ordine Consegnato! +" << finalReward << " Monete.";
                        if (goldBoostTimer > 0.0f) std::cout << " (Bonus Bardo x2!)";
                        std::cout << " | Incasso Totale: " << score << std::endl;

                        if (customersServed > 0 && customersServed % 6 == 0) {
                            player.activateSpeedBoost(6.0f);
                            std::cout << "\n[COMBO] Hai servito 6 piatti! VELOCITA' AUMENTATA!" << std::endl;
                        }

                        if (customersServed > 0 && customersServed % 5 == 0) {
                            currentLevel++;
                            std::cout << "\n====================================" << std::endl;
                            std::cout << "*** LEVEL UP! LIVELLO " << currentLevel << " ***" << std::endl;
                            if (currentSpawnRate > 2.0f) currentSpawnRate -= 2.0f;
                            if (currentMaxPatience > 15.0f) currentMaxPatience -= 5.0f;
                        }

                        if (rand() % 100 < 40) {
                            dirts.push_back(Dirt(custPos));
                            checkTavernDirt();
                        }

                        player.setHeldItem(-1);
                        customers.erase(customers.begin() + i);
                        delivered = true;
                        break;
                    }
                }
            }
        }
    }
    spacebarPressed = spaceIsPressed;

    if (cIsPressed && !cWasPressed) {
        if (player.getHeldItem() != -1) {
            std::cout << "Piatto buttato via!" << std::endl;
            player.setHeldItem(-1);
        }
    }
    cWasPressed = cIsPressed;

    // Pulizia (E)
    bool eIsPressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    if (eIsPressed && !eWasPressed) {
        bool cleanedSomething = false;

        for (int i = 0; i < (int)dirts.size(); i++) {
            if (glm::distance(player.getPos(), dirts[i].getPos()) < 60.0f) {
                dirts.erase(dirts.begin() + i);
                std::cout << "Hai spazzato via una macchia!" << std::endl;
                checkTavernDirt();
                cleanedSomething = true;
                break;
            }
        }

        if (!cleanedSomething) {
            std::cout << "Niente da pulire qui vicino." << std::endl;
        }
    }
    eWasPressed = eIsPressed;

    if (wizard.isWaitingForAnswer()) {

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            if (score >= 15) {
                score -= 15;
                dirts.clear();
                wizard.answerPrompt(true);
                std::cout << "Monete rimanenti: " << score << std::endl;
                checkTavernDirt();
            }
            else {
                std::cout << "Non hai 15 monete! Il mago ti guarda male e se ne va." << std::endl;
                wizard.answerPrompt(false);
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            std::cout << "Hai rifiutato l'offerta." << std::endl;
            wizard.answerPrompt(false);
        }
    }
}