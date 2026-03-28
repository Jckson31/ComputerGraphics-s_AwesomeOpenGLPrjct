#include "Game.h"
#include <glm/ext/matrix_transform.hpp>

// --- COSTRUTTORE ---
Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Width(width), Height(height)
{
    score = 0;
    currentLevel = 1;
    customersServed = 0;
    angryCustomers = 0;

    currentSpawnRate = 10.0f;
    currentMaxPatience = 50.0f;

    randomDirtTimer = 0.0f;
    spawnTimer = 0.0f;
    speedBoostTimer = 0.0f;
    goldBoostTimer = 0.0f;

    isGameOver = false;

    spacebarPressed = false;
    cWasPressed = false;
    eWasPressed = false;

    player = nullptr;
    bard = nullptr;
    wizard = nullptr;
    counter = nullptr;
    trashCan = nullptr;
}

// --- DISTRUTTORE ---
Game::~Game() {
    delete player;
    delete bard;
    delete wizard;
    delete counter;
    delete trashCan;
}

// --- INIT (Prepara la scena) ---
void Game::Init() {
    std::cout << "Inizializzazione Taverna in corso..." << std::endl;

    player = new Player(glm::vec2(400.0f, 350.0f));
    bard = new Bard(glm::vec2(40.0f, 60.0f));
    wizard = new Wizard(glm::vec2(40.0f, 60.0f));

    counter = new TavernObject(glm::vec2(350.0f, 450.0f), glm::vec2(600.0f, 100.0f), glm::vec3(0.6f, 0.3f, 0.1f));
    trashCan = new TavernObject(glm::vec2(750.0f, 500.0f), glm::vec2(30.0f, 40.0f), glm::vec3(0.3f, 0.3f, 0.3f));

    tables.clear();

    std::vector<glm::vec2> sideSeats = {
        glm::vec2(0.0f, 70.0f), glm::vec2(0.0f, -15.0f),
        glm::vec2(-80.0f, 30.0f), glm::vec2(80.0f, 30.0f)
    };

    std::vector<glm::vec2> centerSeats = {
        glm::vec2(-40.0f, 70.0f), glm::vec2(40.0f, 70.0f),
        glm::vec2(-40.0f, -15.0f), glm::vec2(40.0f, -15.0f),
        glm::vec2(-100.0f, 30.0f), glm::vec2(100.0f, 30.0f)
    };

    tables.push_back(TavernObject(glm::vec2(150.0f, 300.0f), glm::vec2(120.0f, 40.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(150.0f, 100.0f), glm::vec2(120.0f, 40.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(650.0f, 300.0f), glm::vec2(120.0f, 40.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(650.0f, 100.0f), glm::vec2(120.0f, 40.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(400.0f, 200.0f), glm::vec2(160.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), centerSeats));

    foodStations.clear();
    foodStations.push_back(FoodStation(glm::vec2(100.0f, 500.0f), glm::vec2(40.0f, 40.0f), 0));
    foodStations.push_back(FoodStation(glm::vec2(265.0f, 500.0f), glm::vec2(40.0f, 40.0f), 1));
    foodStations.push_back(FoodStation(glm::vec2(435.0f, 500.0f), glm::vec2(40.0f, 40.0f), 2));
    foodStations.push_back(FoodStation(glm::vec2(600.0f, 500.0f), glm::vec2(40.0f, 40.0f), 3));
}

// --- UPDATE (Logica che avanza nel tempo) ---
void Game::Update(float dt) {
    if (isGameOver) return;

    // 1. Logica Sporco
    randomDirtTimer += dt;
    if (randomDirtTimer > 10.0f) {
        randomDirtTimer = 0.0f;
        glm::vec2 dirtPos;
        bool positionIsValid = false;
        int attempts = 0;

        // Continua a cercare un posto finché non ne trova uno libero 
        // (max 15 tentativi per evitare che il gioco si blocchi in un loop infinito)
        while (!positionIsValid && attempts < 15) {
            float randomX = 50.0f + (rand() % 700);
            float randomY = 50.0f + (rand() % 400); // Fino a 400 così non va dietro al bancone
            dirtPos = glm::vec2(randomX, randomY);
            glm::vec2 dirtSize = glm::vec2(40.0f, 40.0f); // Scatola di sicurezza della macchia

            positionIsValid = true; // Presumiamo sia valido, poi cerchiamo collisioni

            // A. Controlla se la macchia finisce sul bancone
            if (CheckCollision(dirtPos, dirtSize, counter->getPos(), glm::vec2(600.0f, 100.0f))) {
                positionIsValid = false;
            }

            // B. Controlla se la macchia finisce sotto un tavolo
            for (int i = 0; i < tables.size(); i++) {
                // Il tavolo grande ha una scatola più grande
                glm::vec2 tSize = (i == tables.size() - 1) ? glm::vec2(160.0f, 80.0f) : glm::vec2(130.0f, 80.0f);

                if (CheckCollision(dirtPos, dirtSize, tables[i].getPos(), tSize)) {
                    positionIsValid = false;
                    break; // Inutile controllare gli altri tavoli, questo tentativo è fallito
                }
            }
            attempts++;
        }

        // Se dopo i tentativi ha trovato un posto libero, genera lo sporco!
        if (positionIsValid) {
            dirts.push_back(Dirt(dirtPos));
            std::cout << "E' apparsa una macchia sul pavimento..." << std::endl;
            PrintDashboard();
        }
    }

    // 2. Aggiornamento Player
    player->update(dt);

    // 3. Aggiornamento Bardo e Mago
    if (goldBoostTimer > 0.0f) {
        goldBoostTimer -= dt;
        if (goldBoostTimer <= 0.0f) {
            bard->stopPlaying();
            std::cout << "Il bardo ha TERMINATO l'esibizione e se ne va." << std::endl;
        }
    }
    bard->update(dt, wizard->isVisible());
    wizard->update(dt, bard->isVisible());

    // 4. Spawn Clienti
    spawnTimer += dt;
    if (spawnTimer > currentSpawnRate) {
        spawnTimer = 0.0f;

        struct FreeSeat { int tId; int sId; };
        std::vector<FreeSeat> freeSeats;

        for (int i = 0; i < tables.size(); i++) {
            for (int j = 0; j < tables[i].getSeatCount(); j++) {
                bool isFree = true;
                for (int k = 0; k < customers.size(); k++) {
                    if (customers[k].table == i && customers[k].seat == j) {
                        isFree = false; break;
                    }
                }
                if (isFree) freeSeats.push_back({ i, j });
            }
        }

        if (freeSeats.size() > 0) {
            int randomIndex = rand() % freeSeats.size();
            FreeSeat chosen = freeSeats[randomIndex];
            glm::vec2 startPos(400.0f, -20.0f);
            glm::vec2 target = tables[chosen.tId].getSeatPos(chosen.sId);
            int randomFood = rand() % 4;

            customers.push_back(Customer(chosen.tId, chosen.sId, startPos, target, randomFood, currentMaxPatience));
        }
    }

    // 5. Aggiornamento Clienti
    for (int i = 0; i < customers.size(); ) {
        customers[i].update(dt, bard->isPlaying());

        if (customers[i].isAngry && !customers[i].getIsLeaving()) {
            angryCustomers++;
            std::cout << "Un cliente se n'e' andato arrabbiato! -5 Monete!" << std::endl;
            score -= 5;
            if (rand() % 100 < 40) {
                dirts.push_back(Dirt(customers[i].getPos()));
                PrintDashboard();
            }
            customers[i].leave();
        }

        if (customers[i].getHasLeft()) {
            customers.erase(customers.begin() + i);
        }
        else {
            i++;
        }
    }
}

// --- PROCESS INPUT (Tastiera e collisioni) ---
void Game::ProcessInput(GLFWwindow* window, float dt) {
    if (isGameOver) return;

    // Movimento Player
    glm::vec2 moveDir = player->calculateMovement(window, dt);
    glm::vec2 playerPos = player->getPos();
    glm::vec2 playerSize = player->getSize();

    float spessoreMuro = 70.0f;

    glm::vec2 nextPosX = playerPos; nextPosX.x += moveDir.x;
    glm::vec2 nextPosY = playerPos; nextPosY.y += moveDir.y;

    float halfWidth = playerSize.x / 2.0f;
    float halfHeight = playerSize.y / 2.0f;

    if (nextPosX.x - halfWidth < 0.0f) nextPosX.x = halfWidth;
    if (nextPosX.x + halfWidth > Width) nextPosX.x = Width - halfWidth;
    if (nextPosY.y - halfHeight < 0.0f) nextPosY.y = halfHeight;
    if (nextPosY.y + halfHeight > (Height - spessoreMuro)) nextPosY.y = (Height - spessoreMuro) - halfHeight;

    // Collisioni Asse X
    bool colX = false;
    glm::vec2 counterCollPos = glm::vec2(350.0f, counter->getPos().y + 10.0f); // Leggermente abbassato
    glm::vec2 counterCollSize = glm::vec2(600.0f, 40.0f);
    if (CheckCollision(nextPosX, playerSize, counterCollPos, counterCollSize)) colX = true;
    if (CheckCollision(nextPosX, playerSize, trashCan->getPos(), trashCan->getSize())) colX = true;

    // SCATOLE PIU' LARGHE, MA PIU' SOTTILI IN ALTEZZA
    glm::vec2 tableCollSize = glm::vec2(120.0f, 30.0f);
    glm::vec2 bigTableCollSize = glm::vec2(160.0f, 40.0f);

    for (int i = 0; i < tables.size() - 1; i++) {
        // Offset Y ridotto a +5.0f per far avvicinare l'oste di più dal davanti
        glm::vec2 tableCollPos = tables[i].getPos() + glm::vec2(0.0f, 5.0f);
        if (CheckCollision(nextPosX, playerSize, tableCollPos, tableCollSize)) colX = true;
    }
    glm::vec2 bigTableCollPos = tables[tables.size() - 1].getPos() + glm::vec2(0.0f, 5.0f);
    if (CheckCollision(nextPosX, playerSize, bigTableCollPos, bigTableCollSize)) colX = true;

    if (!colX) playerPos.x = nextPosX.x;

    // Collisioni Asse Y
    bool colY = false;
    if (CheckCollision(nextPosY, playerSize, counterCollPos, counterCollSize)) colY = true;
    if (CheckCollision(nextPosY, playerSize, trashCan->getPos(), trashCan->getSize())) colY = true;

    for (int i = 0; i < tables.size() - 1; i++) {
        glm::vec2 tableCollPos = tables[i].getPos() + glm::vec2(0.0f, 5.0f);
        if (CheckCollision(nextPosY, playerSize, tableCollPos, tableCollSize)) colY = true;
    }
    if (CheckCollision(nextPosY, playerSize, bigTableCollPos, bigTableCollSize)) colY = true;

    if (!colY) playerPos.y = nextPosY.y;

    player->setPos(playerPos);

    // Azioni con SPAZIO
    bool spaceIsPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (spaceIsPressed && !spacebarPressed) {
        bool interactedWithNPC = false;

        if (bard->tryInteract(player->getPos())) {
            goldBoostTimer = 15.0f;
            interactedWithNPC = true;
            std::cout << "\n BONUS ATTIVO! Il Bardo suona!\n" << std::endl;
        }

        if (!interactedWithNPC && wizard->tryInteract(player->getPos())) {
            interactedWithNPC = true;
            std::cout << "\n MAGO: 'Per 15 monete d'oro, faro' sparire ogni macchia! Accetti?'  \n";
        }

        if (!interactedWithNPC) {
            if (player->getHeldItem() == -1) {
                for (int i = 0; i < foodStations.size(); i++) {
                    if (glm::distance(player->getPos(), foodStations[i].getPos()) < 120.0f) {
                        player->setHeldItem(i);
                        break;
                    }
                }
            }
            else {
                for (int i = 0; i < customers.size(); i++) {
                    glm::vec2 custPos = tables[customers[i].table].getSeatPos(customers[i].seat);
                    if (glm::distance(player->getPos(), custPos) < 120.0f &&
                        customers[i].waiting && !customers[i].getIsWalking() &&
                        customers[i].desiredFood == player->getHeldItem())
                    {
                        int baseReward = (goldBoostTimer > 0.0f) ? 20 : 10;
                        float pazienza = customers[i].getPatience();

                        if (pazienza <= 10.0f) baseReward /= 2;
                        else if (pazienza <= 20.0f) baseReward = (int)(baseReward * 0.75f);

                        int finalReward = baseReward - dirts.size();
                        if (finalReward < 1) finalReward = 1;

                        score += finalReward;
                        customersServed++;

                        if (customersServed % 6 == 0) player->activateSpeedBoost(6.0f);
                        if (customersServed % 5 == 0) {
                            currentLevel++;
                            angryCustomers = 0;
                            if (currentSpawnRate > 2.0f) currentSpawnRate -= 2.0f;
                            if (currentMaxPatience > 15.0f) currentMaxPatience -= 5.0f;
                        }

                        if (rand() % 100 < 40) {
                            dirts.push_back(Dirt(custPos));
                            PrintDashboard();
                        }

                        player->setHeldItem(-1);
                        customers[i].servedFood();
                        break;
                    }
                }
            }
        }
    }
    spacebarPressed = spaceIsPressed;

    // Azione Buttare (C)
    bool cIsPressed = (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);
    if (cIsPressed && !cWasPressed) {
        if (player->getHeldItem() != -1 && glm::distance(player->getPos(), trashCan->getPos()) < 80.0f) {
            player->setHeldItem(-1);
        }
    }
    cWasPressed = cIsPressed;

    // Azione Pulire (E)
    bool eIsPressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    if (eIsPressed && !eWasPressed) {
        for (int i = 0; i < dirts.size(); i++) {
            if (glm::distance(player->getPos(), dirts[i].getPos()) < 60.0f) {
                dirts.erase(dirts.begin() + i);
                PrintDashboard();
                break;
            }
        }
    }
    eWasPressed = eIsPressed;

    // Risposte al Mago
    if (wizard->isWaitingForAnswer()) {
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            if (score >= 15) {
                score -= 15;
                dirts.clear();
                wizard->answerPrompt(true);
                PrintDashboard();
            }
            else {
                wizard->answerPrompt(false);
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            wizard->answerPrompt(false);
        }
    }
}

// --- UTILITÀ ---
bool Game::CheckCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2) {
    bool collisionX = (pos1.x + size1.x / 2.0f) >= (pos2.x - size2.x / 2.0f) &&
        (pos1.x - size1.x / 2.0f) <= (pos2.x + size2.x / 2.0f);
    bool collisionY = (pos1.y + size1.y / 2.0f) >= (pos2.y - size2.y / 2.0f) &&
        (pos1.y - size1.y / 2.0f) <= (pos2.y + size2.y / 2.0f);
    return collisionX && collisionY;
}

void Game::PrintDashboard() {
    int currentDirt = dirts.size();
    std::cout << "\n================ STATO TAVERNA ================" << std::endl;
    std::cout << " SPORCIZIA: [";
    for (int i = 0; i < MAX_DIRT; i++) {
        if (i < currentDirt) std::cout << "#"; else std::cout << "-";
    }
    std::cout << "] " << currentDirt << "/" << MAX_DIRT << std::endl;

    std::cout << " CLIENTI ANDATI VIA:    [";
    for (int i = 0; i < MAX_ANGRY_CUSTOMERS; i++) {
        if (i < angryCustomers) std::cout << "X"; else std::cout << "-";
    }
    std::cout << "] " << angryCustomers << "/" << MAX_ANGRY_CUSTOMERS << std::endl;
    std::cout << "===============================================\n" << std::endl;

    if (currentDirt >= MAX_DIRT && !isGameOver) {
        isGameOver = true;
        std::cout << "!!!!!!!!!!!!!!!! GAME OVER !!!!!!!!!!!!!!!!" << std::endl;
        std::cout << " L'ispettore sanitario ha chiuso il locale " << std::endl;
    }
}

void Game::drawTavernModel(Shader& shader, Model& model, float screenX, float screenY, float scaleX, float scaleY, float scaleZ, float rotX, float rotY) {
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, glm::vec3(screenX, screenY, (Height - screenY))); // Uso Height invece di SCR_HEIGHT
    mat = glm::rotate(mat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::scale(mat, glm::vec3(scaleX, scaleY, scaleZ));
    shader.setMat4("model", mat);
    model.Draw(shader);
}


// ==== RENDER ====

void Game::Render(Shader& spriteShader, Shader& modelShader, Model& tableModel, Model& counterModel) {

    // ---- MODELLI 3D (BANCONI E TAVOLI) ---- 
    modelShader.use();
    glEnable(GL_DEPTH_TEST);

    // --- BANCONE ---
    modelShader.setBool("hasDiffuseTexture", true);
    //modelShader.setVec3("flatColor", glm::vec3(0.55f, 0.28f, 0.08f));
    glActiveTexture(GL_TEXTURE0); // Usiamo lo slot 0 per semplicità
    glBindTexture(GL_TEXTURE_2D, texCounter);
    drawTavernModel(
        modelShader, counterModel,
        counter->getPos().x, counter->getPos().y,
        120.0f, 60.0f, 60.0f,   // Partiamo da una scala uniforme molto più piccola
        -30.0f, 180.0f                 // L'angolo per vederne il fronte (2.5D),
    );

    // --- TUTTI I TAVOLI ---
    int Ntables = tables.size();

    modelShader.setBool("hasDiffuseTexture", true);
    //modelShader.setVec3("flatColor", glm::vec3(0.40f, 0.20f, 0.08f));
    glActiveTexture(GL_TEXTURE0); // Usiamo lo slot 0 per semplicità
    glBindTexture(GL_TEXTURE_2D, texTable);
    modelShader.setInt("texture_diffuse1", 0);


    for (int i = 0; i < Ntables - 1; i++) {
        drawTavernModel(
            modelShader, tableModel,
            tables[i].getPos().x, tables[i].getPos().y,
            50.0f, 55.0f, 50.0f,   // Scala di 35 per i tavoli (prima usavi 160!)
            35.0f, 0.0f                // L'angolo per vederne le gambe (2.5D)
        );
    }

    drawTavernModel(
        modelShader, tableModel,
        tables[Ntables - 1].getPos().x, tables[Ntables - 1].getPos().y,
        70.0f, 60.0f, 60.0f,   // Scala di 35 per i tavoli (prima usavi 160!)
        35.0f, 0.0f                // L'angolo per vederne le gambe (2.5D)
    );

    spriteShader.use();
    glActiveTexture(GL_TEXTURE0);

    // === PAVIMENTO ===
    glBindTexture(GL_TEXTURE_2D, texFloor);
    spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

    // Il pavimento è seamless, quindi lo facciamo ripetere per evitare che i pixel sembrino giganti
    spriteShader.setVec2("texScale", 4.0f, 3.0f);
    spriteShader.setVec2("texOffset", 0.0f, 0.0f);
    spriteShader.setBool("usePaletteSwap", false);

    glm::mat4 modelFloor = glm::mat4(1.0f);
    // Posizioniamo il pavimento a Z=0.0f (sul fondo)
    modelFloor = glm::translate(modelFloor, glm::vec3(Width / 2.0f, Height / 2.0f, 0.0f));
    // Lo scaliamo per coprire l'intera larghezza e altezza dello schermo
    modelFloor = glm::scale(modelFloor, glm::vec3(Width, Height, 1.0f));
    spriteShader.setMat4("model", modelFloor);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // === MURO ===
    glBindTexture(GL_TEXTURE_2D, texWall);
    // Il muro non si ripete in altezza, ma un pochino in larghezza
    spriteShader.setVec2("texScale", 2.0f, 1.0f);

    glm::mat4 modelWall = glm::mat4(1.0f);
    // Posizioniamo il muro in alto (Width/2, Height - 50.0f) e leggermente davanti al pavimento (Z=0.01f)
    modelWall = glm::translate(modelWall, glm::vec3(Width / 2.0f, Height - 50.0f, 0.01f));
    // Creiamo una fascia alta 100 pixel per tutta la larghezza dello schermo
    modelWall = glm::scale(modelWall, glm::vec3(Width, 100.0f, 1.0f));
    spriteShader.setMat4("model", modelWall);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Resettiamo la scala della texture a (1.0f, 1.0f) per gli oggetti successivi (cestino, oste, ecc.)
    spriteShader.setVec2("texScale", 1.0f, 1.0f);

    // ===== CESTINO =====
    spriteShader.setVec2("texScale", 1.0f, 1.0f);
    spriteShader.setVec2("texOffset", 0.0f, 0.0f);
    spriteShader.setBool("usePaletteSwap", false);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texTrash);
    spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 modelTrash = glm::mat4(1.0f);
    modelTrash = glm::translate(modelTrash, glm::vec3(trashCan->getPos().x, trashCan->getPos().y, 0.0f));
    modelTrash = glm::scale(modelTrash, glm::vec3(trashCan->getSize().x, trashCan->getSize().y, 2.0f));
    spriteShader.setMat4("model", modelTrash);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // ===== GIOCATORE (OSTE) =====
    spriteShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, playerTextures[player->getCurrentDir()]);  //usa la texture in base alla direzione del player
    spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

    spriteShader.setVec2("texScale", 0.5f, 1.0f);   // Ogni animazione ha 2 frame, quindi scalare la texture orizzontalmente di 0.5 per mostrare solo un frame alla volta
    float xOffset = player->getCurrentFrame() * 0.5f; // Calcola l'offset orizzontale in base al frame corrente (0 o 1)
    spriteShader.setVec2("texOffset", xOffset, 0.0f); // Applica l'offset alla shader per mostrare il frame corretto dell'animazione 

    // RIDIMENSIONAMENTO GEOMETRICO DINAMICO
    int dir = player->getCurrentDir();
    float drawW, drawH;

    //currentDir: 0=Giù, 1=Su, 2=Sinistra, 3=Destra
    if (dir == 2 || dir == 3) {
        // --- CAMMINATA LATERALE (Destra/Sinistra) ---
        // PROVA A CAMBIARE QUESTI VALORI FINCHE' LA PROPORZIONE NON SEMBRA GIUSTA!
        // Esempio: se è troppo grasso, diminuisci drawW. Se è troppo basso, aumenta drawH.
        drawW = 40.0f; // Esempio: lo allarghiamo un po' geometricamente per accogliere lo sprite largo
        drawH = 80.0f; // Esempio: manteniamo l'altezza orizzontale corretta per lo sprite
        // *Nota: Se il tuo sprite laterale è nativamente quasi quadrato, 
        // potresti voler usare, per esempio, drawW=50.0f; drawH=50.0f;
    }
    else if (dir == 1) {
        drawW = 40.0f; // Valore base (40)
        drawH = 80.0f; // Valore base (65) aumentato per la camminata verso l'alto
    }
    else {
        // --- CAMMINATA VERTICALE (Su/Giu) ---
        // Usiamo il valore base snello e alto che abbiamo impostato nel Player.cpp (es. 40x65)
        drawW = player->getSize().x; // Valore base (40)
        drawH = player->getSize().y; // Valore base (65)
    }
    // --------------------------------------------------------------

    glm::mat4 modelPlayer = glm::mat4(1.0f);
    modelPlayer = glm::translate(modelPlayer, glm::vec3(player->getPos().x, player->getPos().y, (Height - player->getPos().y) + 30.0f));

    // USIAMO I VALORI CALCOLATI DINAMICAMENTE PER LA SCALA!
    modelPlayer = glm::scale(modelPlayer, glm::vec3(drawW, drawH, 1.0f));

    spriteShader.setMat4("model", modelPlayer);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Resettiamo a 1.0 per gli altri oggetti
    spriteShader.setVec2("texScale", 1.0f, 1.0f);
    spriteShader.setVec2("texOffset", 0.0f, 0.0f);


    // ===== DISEGNA IL CIBO IN MANO =====
    int currentItem = player->getHeldItem(); // Leggiamo cosa ha in mano
    if (currentItem != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, foodTextures[currentItem]);

        spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));
        glm::mat4 modelHeld = glm::mat4(1.0f);
        modelHeld = glm::translate(modelHeld, glm::vec3(player->getPos().x, player->getPos().y + 40.0f, (Height - player->getPos().y) + 33.0f));
        modelHeld = glm::scale(modelHeld, glm::vec3(20.0f, 20.0f, 1.0f));
        spriteShader.setMat4("model", modelHeld);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ===== STAZIONI =====
    for (int i = 0; i < foodStations.size(); i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, foodTextures[i]);

        // Colore BIANCO per non alterare i colori della tua Pixel Art!
        spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

        glm::mat4 modelStation = glm::mat4(1.0f);
        modelStation = glm::translate(modelStation, glm::vec3(foodStations[i].getPos().x, foodStations[i].getPos().y + 15.0f, 500.0f));
        modelStation = glm::scale(modelStation, glm::vec3(foodStations[i].getSize().x, foodStations[i].getSize().y, 1.0f));
        spriteShader.setMat4("model", modelStation);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ===== DISEGNA LO SPORCO =====
    for (int i = 0; i < dirts.size(); i++) {
        // Colore verdastro/marrone scuro per lo sporco
        spriteShader.setVec3("noteColor", glm::vec3(0.3f, 0.4f, 0.1f));
        glm::mat4 modelDirt = glm::mat4(1.0f);

        // Lo mettiamo a Z=0.05f (sotto i tavoli ma sopra il pavimento)
        modelDirt = glm::translate(modelDirt, glm::vec3(dirts[i].getPos().x, dirts[i].getPos().y, 0.05f));
        modelDirt = glm::scale(modelDirt, glm::vec3(dirts[i].getSize().x, dirts[i].getSize().y, 1.0f));
        spriteShader.setMat4("model", modelDirt);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    /// ===== DISEGNA IL BARDO =====
    if (bard->isVisible()) {
        spriteShader.use();
        glActiveTexture(GL_TEXTURE0);

        // 1. Rimuoviamo il vecchio filtro viola: ora usa i tuoi colori!
        spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

        // 2. Impostiamo l'animazione (Scale a 0.5 perché ha 2 frame)
        spriteShader.setVec2("texScale", 0.5f, 1.0f);
        float bOffset = bard->getCurrentFrame() * 0.5f;
        spriteShader.setVec2("texOffset", bOffset, 0.0f);

        // 3. Scegliamo l'immagine in base a cosa sta facendo
        int bState = bard->getActionState();
        if (bState == 2) glBindTexture(GL_TEXTURE_2D, texBardPlay);
        else if (bState == 1) glBindTexture(GL_TEXTURE_2D, texBardUp);
        else glBindTexture(GL_TEXTURE_2D, texBardDown);

        glm::mat4 modelBard = glm::mat4(1.0f);
        modelBard = glm::translate(modelBard, glm::vec3(bard->getPos().x, bard->getPos().y, 0.12f));
        modelBard = glm::scale(modelBard, glm::vec3(bard->getSize().x, bard->getSize().y, 1.0f));
        spriteShader.setMat4("model", modelBard);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 4. RESET per gli altri oggetti
        spriteShader.setVec2("texScale", 1.0f, 1.0f);
        spriteShader.setVec2("texOffset", 0.0f, 0.0f);
    }

    // ===== DISEGNA IL MAGO =====
    if (wizard->isVisible()) {
        spriteShader.use();
        glActiveTexture(GL_TEXTURE0);

        // 1. Rimuoviamo il vecchio filtro azzurro
        spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

        // 2. Il Mago ha solo 1 frame per file, quindi la finestra è grande tutta l'immagine!
        spriteShader.setVec2("texScale", 1.0f, 1.0f);
        spriteShader.setVec2("texOffset", 0.0f, 0.0f);

        // 3. Scegliamo l'immagine (Su o Giù)
        if (wizard->isEntering()) {
            glBindTexture(GL_TEXTURE_2D, texWizUp);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, texWizDown);
        }

        glm::mat4 modelWizard = glm::mat4(1.0f);
        modelWizard = glm::translate(modelWizard, glm::vec3(wizard->getPos().x, wizard->getPos().y, 0.12f));
        modelWizard = glm::scale(modelWizard, glm::vec3(wizard->getSize().x, wizard->getSize().y, 1.0f));
        spriteShader.setMat4("model", modelWizard);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ===== DISEGNA I CLIENTI E I LORO ORDINI =====
    for (int i = 0; i < customers.size(); i++) {
        glm::vec2 custPos = customers[i].getPos();
        bool isWalking = customers[i].getIsWalking();

        spriteShader.use();
        glActiveTexture(GL_TEXTURE0);

        spriteShader.setBool("usePaletteSwap", true);

        spriteShader.setVec3("originalClothesColor", glm::vec3(0.05f, 0.38f, 0.05f)); // Verde originale
        spriteShader.setVec3("originalHairColor", glm::vec3(0.30f, 0.18f, 0.05f));    // Marrone originale

        spriteShader.setVec3("newHairColor", customers[i].getHairColor());

        int cDir = customers[i].getCurrentDir();
        int cFrame = customers[i].getCurrentFrame();

        // 1. Scegliamo la texture in base alla direzione
        glBindTexture(GL_TEXTURE_2D, custTextures[cDir]);

        // 2. Impostiamo l'animazione (hanno sempre 2 frame, quindi scala 0.5)
        spriteShader.setVec2("texScale", 0.5f, 1.0f);
        spriteShader.setVec2("texOffset", cFrame * 0.5f, 0.0f);

        // 3. COLORE VESTITI E PAZIENZA
        glm::vec3 tint = customers[i].getClothesColor();
        // Se sta perdendo la pazienza, diventa rosso rabbia!
        if (!customers[i].getIsWalking() && customers[i].getPatience() <= 10.0f) {
            // Se è arrabbiato, spegniamo il Palette Swap e lo coloriamo tutto di rosso come facevamo prima!
            spriteShader.setBool("usePaletteSwap", false);
            tint = glm::vec3(1.0f, 0.1f, 0.1f);
        }
        spriteShader.setVec3("noteColor", tint);

        if (!isWalking) {
            // IL CLIENTE E' SEDUTO
            glBindTexture(GL_TEXTURE_2D, texCustSeated); // Usiamo l'immagine singola
            spriteShader.setVec2("texScale", 1.0f, 1.0f); // Finestra intera
            spriteShader.setVec2("texOffset", 0.0f, 0.0f); // Nessuno scorrimento

            drawW = 40.0f; // Proporzioni dell'omino fermo (aggiusta se ti sembra schiacciato)
            drawH = 65.0f;
        }
        else {
            // IL CLIENTE STA CAMMINANDO
            glBindTexture(GL_TEXTURE_2D, custTextures[cDir]); // Usiamo l'array delle direzioni
            spriteShader.setVec2("texScale", 0.5f, 1.0f);        // Finestra a metà
            spriteShader.setVec2("texOffset", cFrame * 0.5f, 0.0f); // Scorrimento animato

            if (cDir == 2 || cDir == 3) {
                drawW = 55.0f; // Più largo di lato
                drawH = 65.0f;
            }
            else {
                drawW = 40.0f; // Normale in su/giù
                drawH = 65.0f;
            }
        }

        glm::mat4 modelCust = glm::mat4(1.0f);
        modelCust = glm::translate(modelCust, glm::vec3(custPos.x, custPos.y, (Height - custPos.y) + 43.0f));
        modelCust = glm::scale(modelCust, glm::vec3(drawW, drawH, 1.0f));
        spriteShader.setMat4("model", modelCust);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 5. RESET per gli altri oggetti
        spriteShader.setBool("usePaletteSwap", false);
        spriteShader.setVec2("texScale", 1.0f, 1.0f);
        spriteShader.setVec2("texOffset", 0.0f, 0.0f);

        // --- DISEGNA L'ORDINE ---
        if (!customers[i].getIsWalking() && customers[i].waiting) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, foodTextures[customers[i].desiredFood]);

            spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));
            glm::mat4 modelOrder = glm::mat4(1.0f);
            modelOrder = glm::translate(modelOrder, glm::vec3(custPos.x, custPos.y + 45.0f, (Height - custPos.y) + 43.0f));
            modelOrder = glm::scale(modelOrder, glm::vec3(25.0f, 25.0f, 1.0f));
            spriteShader.setMat4("model", modelOrder);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        if (customers[i].getIsEating()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, foodTextures[customers[i].desiredFood]);

            spriteShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));
            glm::mat4 modelFood = glm::mat4(1.0f);

            // Lo posizioniamo ad altezza "tavolo" (custPos.y + 5.0f).
            // La coordinata Z è +34.0f, così siamo sicuri che venga disegnato SOPRA al cliente che ha +31.0f
            modelFood = glm::translate(modelFood, glm::vec3(custPos.x, custPos.y + 7.0f, (Height - custPos.y) + 46.0f));

            // Lo facciamo un pochino più piccolo rispetto a quando è nel fumetto (20.0f invece di 25.0f)
            modelFood = glm::scale(modelFood, glm::vec3(20.0f, 20.0f, 1.0f));

            spriteShader.setMat4("model", modelFood);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

}