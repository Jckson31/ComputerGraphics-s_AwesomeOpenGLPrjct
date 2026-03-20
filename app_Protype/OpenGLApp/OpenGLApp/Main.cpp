#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera.h"

#include <iostream>
#include <vector>
#include <cstdlib> // Per rand()
#include <ctime>

#include "Customer.h"
#include "Bard.h"
#include "Player.h"
#include "TavernObjects.h"
#include "FoodStations.h"
#include "Dirt.h"
#include "Wizard.h"

// --- FUNZIONE PER CARICARE LE TEXTURE ---
unsigned int loadTexture(char const* path)  //percorso di un'immagine,
//usa la libreria stb_image per decodificarla e 
//la invia alla memoria della scheda video.
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // stbi_set_flip_vertically_on_load(true); // Decommenta questa riga se le immagini appaiono capovolte

    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        // Controlliamo se l'immagine ha la trasparenza (PNG) o no (JPG)
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //quando ingrandisci l'immagine, mantieni i pixel netti -> pixel art
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





void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void printDashboard();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// player (oste)
Player player(glm::vec2(400.0f, 350.0f));
float playerSpeed = 300.0f;                      // Velocità di movimento in pixel al secondo

// --- VARIABILI DI GIOCO ---
int heldItem = -1;             // -1 significa mani vuote, 0-3 è il cibo
int score = 0;                 // I nostri guadagni!
bool spacebarPressed = false;  // Serve a evitare che tenendo premuto Spazio faccia 100 azioni al secondo
bool cWasPressed = false;      // tasto per buttare piatti (C) 
bool eWasPressed = false;      // tasto di pulizia (E)


// --- VARIABILI DI PROGRESSIONE E DIFFICOLTÀ ---
int currentLevel = 1;
int customersServed = 0;          // Conta quanti piatti abbiamo consegnato
float currentSpawnRate = 10.0f;    // Iniziamo lenti: un cliente ogni 5 secondi
float currentMaxPatience = 50.0f; // Iniziamo facili: 30 secondi di pazienza
int angryCustomers = 0;           // Contatore per il futuro Game Over (es. a 5 si perde)

const int MAX_DIRT = 10;
float randomDirtTimer = 0.0f;  // Timer per far comparire macchie a caso

bool isGameOver = false;                // Memorizza se abbiamo perso
const int MAX_ANGRY_CUSTOMERS = 10;     // Limite massimo di clienti arrabbiati


// --- STRUTTURA SPORCO ---
std::vector<Dirt> dirts; // Lista di tutte le macchie sul pavimento


// postazione per ritirare il cibo
TavernObject counter(glm::vec2(400.0f, 550.0f), glm::vec2(600.0f, 100.0f), glm::vec3(0.6f, 0.3f, 0.1f));
// startPos (centro del bancone), startSize (larghezza e altezza), startColor (marrone)

TavernObject trashCan(glm::vec2(750.0f, 550.0f), glm::vec2(30.0f, 40.0f), glm::vec3(0.3f, 0.3f, 0.3f));

// lista tavoli
std::vector<TavernObject> tables;

// stazioni cibo
std::vector<FoodStation> foodStations;


// --- FUNZIONE DI COLLISIONE AABB ---
// Restituisce true se l'oggetto 1 (es. giocatore) si sovrappone all'oggetto 2 (es. tavolo)
bool checkCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2)
{
    // Calcoliamo la distanza tra i centri
    // Un oggetto si sovrappone se la distanza tra i bordi è minore di 0

    // Controllo asse X: il lato destro di 1 è oltre il lato sinistro di 2 AND il lato sinistro di 1 è prima del lato destro di 2
    bool collisionX = (pos1.x + size1.x / 2.0f) >= (pos2.x - size2.x / 2.0f) &&
        (pos1.x - size1.x / 2.0f) <= (pos2.x + size2.x / 2.0f);

    // Controllo asse Y: il lato alto di 1 è oltre il lato basso di 2 AND il lato basso di 1 è prima del lato alto di 2
    bool collisionY = (pos1.y + size1.y / 2.0f) >= (pos2.y - size2.y / 2.0f) &&
        (pos1.y - size1.y / 2.0f) <= (pos2.y + size2.y / 2.0f);

    // C'è collisione SOLO se si sovrappongono in ENTRAMBI gli assi
    return collisionX && collisionY;
}


std::vector<Customer> customers;
float spawnTimer = 0.0f; // Timer per far apparire nuovi clienti

// --- VARIABILI BONUS E POWER-UP ---
float speedBoostTimer = 0.0f; // Quanto tempo manca alla fine dello scatto
float goldBoostTimer = 0.0f;  // Quanto tempo manca alla fine dei guadagni doppi

Bard bard(glm::vec2(40.0f, 60.0f)); // Creiamo il bardo con la sua grandezza
float bardSpawnTimer = 0.0f; // Timer per farlo comparire

Wizard wizard(glm::vec2(40.0f, 60.0f));


int main()
{
    srand(time(NULL));

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
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
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("shader.vs", "shader.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {

        //coord. vertice     //coord. texture
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

    //vertici comande
    float noteVertices[] = {
        // pos            // tex
        -0.3f,  0.2f, 0.0f,  0.0f, 1.0f,
        -0.3f, -0.2f, 0.0f,  0.0f, 0.0f,
         0.3f, -0.2f, 0.0f,  1.0f, 0.0f,

        -0.3f,  0.2f, 0.0f,  0.0f, 1.0f,
         0.3f, -0.2f, 0.0f,  1.0f, 0.0f,
         0.3f,  0.2f, 0.0f,  1.0f, 1.0f
    };

    //BANCONE E ASSE
    unsigned int cubeVAO, cubeVBO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //NOTE
    unsigned int noteVAO, noteVBO;
    glGenVertexArrays(1, &noteVAO);
    glGenBuffers(1, &noteVBO);

    glBindVertexArray(noteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, noteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(noteVertices), noteVertices, GL_STATIC_DRAW);

    // posizione
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("resources/textures/wood.jpeg", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("resources/textures/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture note.png" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    /*
        // --- INIZIALIZZAZIONE ARREDAMENTO ---
            // La cucina in alto (Bancone allungato per le future stazioni)
        counter = glm::vec2(400.0f, 550.0f);
        counter.size = glm::vec2(600.0f, 60.0f); // Larghezza portata a 600!
        counter.color = glm::vec3(0.8f, 0.4f, 0.1f);
    */
    // Svuotiamo la lista per sicurezza
    tables.clear();

    std::vector<glm::vec2> sideSeats = {
        glm::vec2(0.0f, 45.0f),   // Su
        glm::vec2(0.0f, -45.0f),  // Giù
        glm::vec2(-75.0f, 0.0f),  // Sinistra
        glm::vec2(75.0f, 0.0f)    // Destra
    };

    // Definiamo i 6 posti per il grande tavolo centrale (160x80)
    std::vector<glm::vec2> centerSeats = {
        glm::vec2(-40.0f, 55.0f),  // Su-Sinistra
        glm::vec2(40.0f, 55.0f),   // Su-Destra
        glm::vec2(-40.0f, -55.0f), // Giù-Sinistra
        glm::vec2(40.0f, -55.0f),  // Giù-Destra
        glm::vec2(-95.0f, 0.0f),   // Lato Corto Sinistro
        glm::vec2(95.0f, 0.0f)     // Lato Corto Destro
    };

    // Creiamo i tavoli assegnandogli la lista di sedie corretta
    // Tavoli a SINISTRA
    tables.push_back(TavernObject(glm::vec2(150.0f, 400.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(150.0f, 100.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    // Tavoli a DESTRA
    tables.push_back(TavernObject(glm::vec2(650.0f, 400.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    tables.push_back(TavernObject(glm::vec2(650.0f, 100.0f), glm::vec2(120.0f, 60.0f), glm::vec3(0.4f, 0.2f, 0.1f), sideSeats));
    // Tavolo CENTRALE
    tables.push_back(TavernObject(glm::vec2(400.0f, 250.0f), glm::vec2(160.0f, 80.0f), glm::vec3(0.4f, 0.2f, 0.1f), centerSeats));


    // --- INIZIALIZZAZIONE STAZIONI ---
    foodStations.clear();
    // 1. Birra (Giallo)
    foodStations.push_back(FoodStation(glm::vec2(150.0f, 550.0f), glm::vec2(40.0f, 40.0f), 0));
    // 2. Carne (Rosso scuro)
    foodStations.push_back(FoodStation(glm::vec2(315.0f, 550.0f), glm::vec2(40.0f, 40.0f), 1));
    // 3. Zuppa (Verde)
    foodStations.push_back(FoodStation(glm::vec2(485.0f, 550.0f), glm::vec2(40.0f, 40.0f), 2));
    // 4. Pane/Torta (Beige)
    foodStations.push_back(FoodStation(glm::vec2(650.0f, 550.0f), glm::vec2(40.0f, 40.0f), 3));

    unsigned int texFloor = loadTexture("resources/textures/wood.jpeg");
    unsigned int texPlayer = loadTexture("resources/textures/awesomeface.png");

    unsigned int texBeer = loadTexture("resources/textures/beer.png");
    unsigned int texMeat = loadTexture("resources/textures/meat.png");
    unsigned int texBread = loadTexture("resources/textures/bread.png");
    unsigned int texSoup = loadTexture("resources/textures/soup.png");

    unsigned int texCustomer = loadTexture("resources/textures/awesomeface.png");

    unsigned int foodTextures[4];
    foodTextures[0] = texBeer;   // 0 = Birra
    foodTextures[1] = texMeat;   // 1 = Carne
    foodTextures[2] = texSoup; // 2 = Zuppa (tappabuchi temporaneo)
    foodTextures[3] = texBread;  // 3 = Pane (tappabuchi temporaneo)

    ourShader.use();
    ourShader.setInt("texture1", 0);

    //unsigned int texPlayer = loadTexture("resources/textures/player.png");
    unsigned int texBigTable = loadTexture("resources/textures/BigTable.png");

    // --- CARICAMENTO ANIMAZIONI GIOCATORE ---
    // (Assicurati che i nomi dei file qui sotto corrispondano ai nomi e percorsi esatti dei tuoi png!)
    unsigned int texPlayerDown = loadTexture("resources/textures/player_front.png");
    unsigned int texPlayerUp = loadTexture("resources/textures/player_back.png");
    unsigned int texPlayerLeft = loadTexture("resources/textures/player_left.png");
    unsigned int texPlayerRight = loadTexture("resources/textures/player_right.png");

    // Le salviamo in un array per comodità: Indice 0=Giù, 1=Su, 2=Sinistra, 3=Destra
    unsigned int playerTextures[4] = { texPlayerDown, texPlayerUp, texPlayerLeft, texPlayerRight };

    // Texture del Mago
    unsigned int texWizUp = loadTexture("resources/textures/wizard_back.png");
    unsigned int texWizDown = loadTexture("resources/textures/wizard_front.png");

    // Texture del Bardo
    unsigned int texBardUp = loadTexture("resources/textures/bard_walking_back.png");
    unsigned int texBardDown = loadTexture("resources/textures/bard_walking_front.png");
    unsigned int texBardPlay = loadTexture("resources/textures/bard_playing.png");


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        if (!isGameOver) {
            // --- LOGICA SPORCO CASUALE NEL LOCALE ---
            randomDirtTimer += deltaTime;
            if (randomDirtTimer > 8.0f) { // Ogni 12 secondi (puoi abbassarlo per renderlo più difficile!)
                randomDirtTimer = 0.0f; // Resetta il timer

                // Generiamo coordinate X e Y casuali per la macchia
                // La X va da 50 a 750 (per non farle uscire dai muri laterali)
                // La Y va da 50 a 450 (per non farle finire SOTTO o DIETRO il bancone della cucina)
                float randomX = 50.0f + (rand() % 700);
                float randomY = 50.0f + (rand() % 400);

                dirts.push_back(Dirt(glm::vec2(randomX, randomY)));

                std::cout << "E' apparsa una macchia sul pavimento..." << std::endl;
                printDashboard(); // Aggiorna la barra e controlla il Game Over
            }

            // --- AGGIORNAMENTO TIMER DEI BONUS ---
            // --- AGGIORNAMENTO DEL GIOCATORE ---
            player.update(deltaTime); // Il giocatore gestisce da solo il suo scatto!

            // --- AGGIORNAMENTO TIMER BARDO ---
            if (goldBoostTimer > 0.0f) {
                goldBoostTimer -= deltaTime;
                if (goldBoostTimer <= 0.0f) {
                    bard.stopPlaying(); // Diciamo al bardo di smettere!
                    std::cout << "Il bardo ha TERMINATO l'esibizione e se ne va." << std::endl;
                }
            }

            // Il Bardo pensa e si muove tutto da solo!
            bard.update(deltaTime, wizard.isVisible());
            wizard.update(deltaTime, bard.isVisible());





            // --- LOGICA SPAWN CLIENTI ---
            spawnTimer += deltaTime;
            if (spawnTimer > currentSpawnRate) { // <-- MODIFICA QUI
                spawnTimer = 0.0f;

                // Creiamo una piccola struttura temporanea per salvare le sedie libere
                struct FreeSeat { int tId; int sId; };
                std::vector<FreeSeat> freeSeats;

                // Controlliamo ogni tavolo e ogni sedia
                for (int i = 0; i < tables.size(); i++) {
                    for (int j = 0; j < tables[i].getSeatCount(); j++) {
                        bool isFree = true;
                        // Controlliamo se qualche cliente è già seduto qui
                        for (int k = 0; k < customers.size(); k++) {
                            if (customers[k].table == i && customers[k].seat == j) {
                                isFree = false;
                                break;
                            }
                        }
                        if (isFree) {
                            freeSeats.push_back({ i, j }); // Salviamo sia l'ID del tavolo che quello della sedia
                        }
                    }
                }

                if (freeSeats.size() > 0) {
                    int randomIndex = rand() % freeSeats.size();
                    FreeSeat chosen = freeSeats[randomIndex];

                    glm::vec2 startPos(400.0f, -20.0f);
                    glm::vec2 target = tables[chosen.tId].getSeatPos(chosen.sId);
                    int randomFood = rand() % 4;

                    // Creiamo l'oggetto usando il Costruttore!
                    Customer newCust(chosen.tId, chosen.sId, startPos, target, randomFood, currentMaxPatience);

                    customers.push_back(newCust);

                }
            }

            // --- LOGICA MOVIMENTO E PAZIENZA CLIENTI ---
            for (int i = 0; i < customers.size(); ) {

                // Il cliente pensa da solo!
                customers[i].update(deltaTime, bard.isPlaying());

                // Controlliamo solo se ha finito la pazienza
                if (customers[i].isAngry) {
                    angryCustomers++;
                    std::cout << "Un cliente se n'e' andato arrabbiato! -5 Monete!" << std::endl;
                    score -= 5;

                    if (rand() % 100 < 40) {
                        dirts.push_back(Dirt(customers[i].getPos()));
                        printDashboard();
                    }
                    customers.erase(customers.begin() + i);
                }
                else {
                    i++;
                }
            }
        }

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);

        // activate shader
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = glm::mat4(1.0f);   // matrice identità, la telecamera è fissa e guarda lungo -Z, quindi non serve trasformare nulla
        ourShader.setMat4("view", view);


        /*
         //render BANCO and ASSE
         glBindVertexArray(cubeVAO);

         glm::mat4 model;

         ourShader.setBool("useSecondTexture", true);

         // ===== BANCO =====
         glm::vec3 bancoPos = glm::vec3(400.0f, 300.0f, 0.0f); // Al centro dello schermo (800/2, 600/2)
         glm::vec3 bancoScale = glm::vec3(200.0f, 50.0f, 1.0f); // Largo 200 pixel, alto 50

         model = glm::mat4(1.0f);
         model = glm::translate(model, bancoPos);
         model = glm::scale(model, bancoScale);
         ourShader.setMat4("model", model);
         glDrawArrays(GL_TRIANGLES, 0, 36);


         // ===== ASSE =====
         glm::vec3 assePos = glm::vec3(0.0f, 0.0f, -0.2f);
         glm::vec3 asseScale = glm::vec3(3.0f, 0.1f, 0.2f);

         model = glm::mat4(1.0f);
         model = glm::translate(model, assePos);
         model = glm::scale(model, asseScale);
         ourShader.setMat4("model", model);
         glDrawArrays(GL_TRIANGLES, 0, 36);

         // metà dimensioni dell'asse
         float asseHalfHeight = asseScale.y * 0.5f; // 0.05
         float asseHalfDepth = asseScale.z * 0.5f; // 0.1

         // piano superiore e bordo frontale
         float asseTopY = assePos.y + asseHalfHeight;
         float asseFrontZ = assePos.z + asseHalfDepth;
         */


        ourShader.use();
        glBindVertexArray(cubeVAO);

        // activate shader
        ourShader.use();

        // ---> IMPORTANTE: Valori predefiniti per TUTTI gli oggetti non animati! <---
        ourShader.setVec2("texScale", 1.0f, 1.0f);
        ourShader.setVec2("texOffset", 0.0f, 0.0f);


        // ===== BANCONE =====
        ourShader.setVec3("noteColor", counter.getColor());
        glm::mat4 modelKitchen = glm::mat4(1.0f);
        modelKitchen = glm::translate(modelKitchen, glm::vec3(counter.getPos().x, counter.getPos().y, 0.0f));
        modelKitchen = glm::scale(modelKitchen, glm::vec3(counter.getSize().x, counter.getSize().y, 1.0f));
        ourShader.setMat4("model", modelKitchen);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ===== CESTINO =====
        ourShader.setVec3("noteColor", trashCan.getColor());
        glm::mat4 modelTrash = glm::mat4(1.0f);
        modelTrash = glm::translate(modelTrash, glm::vec3(trashCan.getPos().x, trashCan.getPos().y, 0.0f));
        modelTrash = glm::scale(modelTrash, glm::vec3(trashCan.getSize().x, trashCan.getSize().y, 1.0f));
        ourShader.setMat4("model", modelTrash);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // ===== TAVOLI =====
        // ciclo for disegna i tavoli uno per uno
        for (int i = 0; i < tables.size(); i++) {
            ourShader.setVec3("noteColor", tables[i].getColor());
            glm::mat4 modelTable = glm::mat4(1.0f);
            modelTable = glm::translate(modelTable, glm::vec3(tables[i].getPos().x, tables[i].getPos().y, 0.0f));
            modelTable = glm::scale(modelTable, glm::vec3(tables[i].getSize().x, tables[i].getSize().y, 1.0f));
            ourShader.setMat4("model", modelTable);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        // ===== GIOCATORE (OSTE) =====
        ourShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, playerTextures[player.getCurrentDir()]);
        ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

        ourShader.setVec2("texScale", 0.5f, 1.0f);
        float xOffset = player.getCurrentFrame() * 0.5f;
        ourShader.setVec2("texOffset", xOffset, 0.0f);

        // ---> NUOVA LOGICA: RIDIMENSIONAMENTO GEOMETRICO DINAMICO <---
        int dir = player.getCurrentDir();
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
            drawW = player.getSize().x; // Valore base (40)
            drawH = player.getSize().y; // Valore base (65)
        }
        // --------------------------------------------------------------

        glm::mat4 modelPlayer = glm::mat4(1.0f);
        modelPlayer = glm::translate(modelPlayer, glm::vec3(player.getPos().x, player.getPos().y, 0.15f));

        // USIAMO I VALORI CALCOLATI DINAMICAMENTE PER LA SCALA!
        modelPlayer = glm::scale(modelPlayer, glm::vec3(drawW, drawH, 1.0f));

        ourShader.setMat4("model", modelPlayer);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Resettiamo a 1.0 per gli altri oggetti
        ourShader.setVec2("texScale", 1.0f, 1.0f);
        ourShader.setVec2("texOffset", 0.0f, 0.0f);


        // ===== DISEGNA IL CIBO IN MANO =====
        int currentItem = player.getHeldItem(); // Leggiamo cosa ha in mano
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

        // ===== STAZIONI =====
        for (int i = 0; i < foodStations.size(); i++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, foodTextures[i]);

            // Colore BIANCO per non alterare i colori della tua Pixel Art!
            ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

            glm::mat4 modelStation = glm::mat4(1.0f);
            modelStation = glm::translate(modelStation, glm::vec3(foodStations[i].getPos().x, foodStations[i].getPos().y, 0.1f));
            modelStation = glm::scale(modelStation, glm::vec3(foodStations[i].getSize().x, foodStations[i].getSize().y, 1.0f));
            ourShader.setMat4("model", modelStation);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ===== DISEGNA LO SPORCO =====
        for (int i = 0; i < dirts.size(); i++) {
            // Colore verdastro/marrone scuro per lo sporco
            ourShader.setVec3("noteColor", glm::vec3(0.3f, 0.4f, 0.1f));
            glm::mat4 modelDirt = glm::mat4(1.0f);

            // Lo mettiamo a Z=0.05f (sotto i tavoli ma sopra il pavimento)
            modelDirt = glm::translate(modelDirt, glm::vec3(dirts[i].getPos().x, dirts[i].getPos().y, 0.05f));
            modelDirt = glm::scale(modelDirt, glm::vec3(dirts[i].getSize().x, dirts[i].getSize().y, 1.0f));
            ourShader.setMat4("model", modelDirt);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        /// ===== DISEGNA IL BARDO =====
        if (bard.isVisible()) {
            ourShader.use();
            glActiveTexture(GL_TEXTURE0);

            // 1. Rimuoviamo il vecchio filtro viola: ora usa i tuoi colori!
            ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

            // 2. Impostiamo l'animazione (Scale a 0.5 perché ha 2 frame)
            ourShader.setVec2("texScale", 0.5f, 1.0f);
            float bOffset = bard.getCurrentFrame() * 0.5f;
            ourShader.setVec2("texOffset", bOffset, 0.0f);

            // 3. Scegliamo l'immagine in base a cosa sta facendo
            int bState = bard.getActionState();
            if (bState == 2) glBindTexture(GL_TEXTURE_2D, texBardPlay);
            else if (bState == 1) glBindTexture(GL_TEXTURE_2D, texBardUp);
            else glBindTexture(GL_TEXTURE_2D, texBardDown);

            glm::mat4 modelBard = glm::mat4(1.0f);
            modelBard = glm::translate(modelBard, glm::vec3(bard.getPos().x, bard.getPos().y, 0.12f));
            modelBard = glm::scale(modelBard, glm::vec3(bard.getSize().x, bard.getSize().y, 1.0f));
            ourShader.setMat4("model", modelBard);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // 4. RESET per gli altri oggetti
            ourShader.setVec2("texScale", 1.0f, 1.0f);
            ourShader.setVec2("texOffset", 0.0f, 0.0f);
        }

        // ===== DISEGNA IL MAGO =====
        if (wizard.isVisible()) {
            ourShader.use();
            glActiveTexture(GL_TEXTURE0);

            // 1. Rimuoviamo il vecchio filtro azzurro
            ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f));

            // 2. Il Mago ha solo 1 frame per file, quindi la finestra è grande tutta l'immagine!
            ourShader.setVec2("texScale", 1.0f, 1.0f);
            ourShader.setVec2("texOffset", 0.0f, 0.0f);

            // 3. Scegliamo l'immagine (Su o Giù)
            if (wizard.isEntering()) {
                glBindTexture(GL_TEXTURE_2D, texWizUp);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, texWizDown);
            }

            glm::mat4 modelWizard = glm::mat4(1.0f);
            modelWizard = glm::translate(modelWizard, glm::vec3(wizard.getPos().x, wizard.getPos().y, 0.12f));
            modelWizard = glm::scale(modelWizard, glm::vec3(wizard.getSize().x, wizard.getSize().y, 1.0f));
            ourShader.setMat4("model", modelWizard);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ===== DISEGNA I CLIENTI E I LORO ORDINI =====
        for (int i = 0; i < customers.size(); i++) {

            glm::vec2 custPos = customers[i].getPos(); // USIAMO IL GETTER

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texCustomer);

            glm::vec3 custColor = glm::vec3(1.0f, 1.0f, 1.0f);

            if (!customers[i].getIsWalking()) { // USIAMO IL GETTER
                if (customers[i].getPatience() > 20.0f) { // USIAMO IL GETTER
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
            modelCust = glm::translate(modelCust, glm::vec3(custPos.x, custPos.y, 0.11f)); // Z = 0.11, sopra i tavoli
            modelCust = glm::scale(modelCust, glm::vec3(40.0f, 40.0f, 1.0f));
            ourShader.setMat4("model", modelCust);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            if (!customers[i].getIsWalking()) { // Mostra il cibo solo se è seduto!
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, foodTextures[customers[i].desiredFood]);

                ourShader.setVec3("noteColor", glm::vec3(1.0f, 1.0f, 1.0f)); // Colore normale
                glm::mat4 modelOrder = glm::mat4(1.0f);

                // Lo posizioniamo 35 pixel SOPRA la testa del cliente
                modelOrder = glm::translate(modelOrder, glm::vec3(custPos.x, custPos.y + 35.0f, 0.12f));
                modelOrder = glm::scale(modelOrder, glm::vec3(25.0f, 25.0f, 1.0f));
                ourShader.setMat4("model", modelOrder);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
/*
bool checkCollision(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2)
{
    // Calcoliamo la distanza tra i centri
    // Un oggetto si sovrappone se la distanza tra i bordi è minore di 0

    // Controllo asse X: il lato destro di 1 è oltre il lato sinistro di 2 AND il lato sinistro di 1 è prima del lato destro di 2
    bool collisionX = (pos1.x + size1.x / 2.0f) >= (pos2.x - size2.x / 2.0f) &&
        (pos1.x - size1.x / 2.0f) <= (pos2.x + size2.x / 2.0f);

    // Controllo asse Y: il lato alto di 1 è oltre il lato basso di 2 AND il lato basso di 1 è prima del lato alto di 2
    bool collisionY = (pos1.y + size1.y / 2.0f) >= (pos2.y - size2.y / 2.0f) &&
        (pos1.y - size1.y / 2.0f) <= (pos2.y + size2.y / 2.0f);

    // C'è collisione SOLO se si sovrappongono in ENTRAMBI gli assi
    return collisionX && collisionY;
}

*/


// --- DASHBOARD DELLA TAVERNA ---
void printDashboard() {
    int currentDirt = dirts.size();

    std::cout << "\n================ STATO TAVERNA ================" << std::endl;

    // 1. Barra dello Sporco
    std::cout << " SPORCIZIA: [";
    for (int i = 0; i < MAX_DIRT; i++) {
        if (i < currentDirt) std::cout << "#"; else std::cout << "-";
    }
    std::cout << "] " << currentDirt << "/" << MAX_DIRT << std::endl;

    // 2. Barra dei Clienti Arrabbiati
    std::cout << " CLIENTI ANDATI VIA:    [";
    for (int i = 0; i < MAX_ANGRY_CUSTOMERS; i++) {
        if (i < angryCustomers) std::cout << "X"; else std::cout << "-";
    }
    std::cout << "] " << angryCustomers << "/" << MAX_ANGRY_CUSTOMERS << std::endl;
    std::cout << "===============================================\n" << std::endl;

    // --- CONTROLLI GAME OVER ---
    if (currentDirt >= MAX_DIRT && !isGameOver) {
        isGameOver = true;
        std::cout << "!!!!!!!!!!!!!!!! GAME OVER !!!!!!!!!!!!!!!!" << std::endl;
        std::cout << " L'ispettore sanitario ha chiuso il locale " << std::endl;
        std::cout << " perche' era diventato un porcile!         " << std::endl;
        std::cout << " Punteggio Finale: " << score << " Monete" << std::endl;
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" << std::endl;
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    if (isGameOver) return; // se c'è game over, blocchiamo tutti gli input

    // 1. Chiediamo al giocatore di calcolare il movimento (e aggiornare le sue animazioni)
    glm::vec2 moveDir = player.calculateMovement(window, deltaTime);

    // Salviamo posizione e dimensioni attuali per comodità
    glm::vec2 playerPos = player.getPos();
    glm::vec2 playerSize = player.getSize();

    // 2. Creiamo le due posizioni future per gli assi separati
    glm::vec2 nextPosX = playerPos;
    nextPosX.x += moveDir.x;

    glm::vec2 nextPosY = playerPos;
    nextPosY.y += moveDir.y;

    // --- CONTROLLO LIMITI SCHERMO ---
    float halfWidth = playerSize.x / 2.0f;
    float halfHeight = playerSize.y / 2.0f;

    if (nextPosX.x - halfWidth < 0.0f) nextPosX.x = halfWidth;
    if (nextPosX.x + halfWidth > SCR_WIDTH) nextPosX.x = SCR_WIDTH - halfWidth;
    if (nextPosY.y - halfHeight < 0.0f) nextPosY.y = halfHeight;
    if (nextPosY.y + halfHeight > SCR_HEIGHT) nextPosY.y = SCR_HEIGHT - halfHeight;

    // --- CONTROLLO COLLISIONI ASSE X ---
    bool colX = false;
    // Controllo contro il bancone (counter) E il cestino (trashCan)
    if (checkCollision(nextPosX, playerSize, counter.getPos(), counter.getSize())) colX = true;
    if (checkCollision(nextPosX, playerSize, trashCan.getPos(), trashCan.getSize())) colX = true;

    // Controllo contro i tavoli
    for (int i = 0; i < tables.size(); i++) {
        if (checkCollision(nextPosX, playerSize, tables[i].getPos(), tables[i].getSize())) colX = true;
    }
    // Se è libero, aggiorniamo la X reale
    if (!colX) playerPos.x = nextPosX.x;

    // --- CONTROLLO COLLISIONI ASSE Y ---
    bool colY = false;
    // Controllo contro il bancone (counter) E il cestino (trashCan)
    if (checkCollision(nextPosY, playerSize, counter.getPos(), counter.getSize())) colY = true;
    if (checkCollision(nextPosY, playerSize, trashCan.getPos(), trashCan.getSize())) colY = true;

    // Controllo contro i tavoli
    for (int i = 0; i < tables.size(); i++) {
        if (checkCollision(nextPosY, playerSize, tables[i].getPos(), tables[i].getSize())) colY = true;
    }
    // Se è libero, aggiorniamo la Y reale
    if (!colY) playerPos.y = nextPosY.y;

    // 3. COMUNICHIAMO AL GIOCATORE LA POSIZIONE FINALE SICURA
    player.setPos(playerPos);



    // --- INTERAZIONE CON SPAZIO ---
    bool spaceIsPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    bool cIsPressed = (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);


    // Controlliamo che il tasto sia stato appena premuto e non tenuto premuto
    if (spaceIsPressed && !spacebarPressed) {
        bool interactedWithNPC = false;

        // PRIORITÀ 1: Il Bardo
        if (bard.tryInteract(player.getPos())) {
            goldBoostTimer = 15.0f;
            interactedWithNPC = true;
            std::cout << "\n💰 BONUS ATTIVO! Il Bardo suona! 💰\n" << std::endl;
        }

        // PRIORITÀ 2: Il Mago
        if (!interactedWithNPC && wizard.tryInteract(player.getPos())) {
            interactedWithNPC = true;
            std::cout << "\n🔮 MAGO: 'Per 15 monete d'oro, faro' sparire ogni macchia! Accetti?' 🔮" << std::endl;
            std::cout << ">>> Premi [Y] per pagare, [N] per rifiutare. <<<\n" << std::endl;
        }


        // Se non abbiamo parlato con nessuno, raccogliamo/consegniamo il cibo
        if (!interactedWithNPC) {


            // CASO 1: MANI VUOTE -> Proviamo a raccogliere il cibo
            if (player.getHeldItem() == -1) {
                for (int i = 0; i < foodStations.size(); i++) {
                    float distanza = glm::distance(player.getPos(), foodStations[i].getPos());
                    // DISTANZA AUMENTATA A 120 per compensare il bancone!
                    if (distanza < 120.0f) {
                        player.setHeldItem(i);
                        std::cout << "Raccolto piatto: " << i << std::endl;
                        break;
                    }
                }
            }
            // CASO 2: ABBIAMO CIBO IN MANO -> Proviamo a consegnarlo
            else {
                bool delivered = false;
                for (int i = 0; i < customers.size(); i++) {
                    int tId = customers[i].table;
                    int sId = customers[i].seat;

                    // CORREZIONE: Usa solo getSeatPos() perché contiene già la posizione assoluta!
                    glm::vec2 custPos = tables[tId].getSeatPos(sId);

                    float distanza = glm::distance(player.getPos(), custPos);

                    if (distanza < 120.0f && customers[i].waiting && customers[i].desiredFood == player.getHeldItem()) {

                        // --- CALCOLO GUADAGNO ---
                        int baseReward = (goldBoostTimer > 0.0f) ? 20 : 10;


                        float pazienza = customers[i].getPatience();

                        // MALUS PER L'ATTESA
                        if (pazienza <= 10.0f) {
                            // Cliente ROSSO: è furioso, paga la metà (-50%)
                            baseReward = baseReward / 2;
                        }
                        else if (pazienza <= 20.0f) {
                            // Cliente ARANCIONE: è scocciato, paga il 25% in meno
                            baseReward = (int)(baseReward * 0.75f);
                        }
                        // Se è > 20.0f (BLU) è felice e paga il 100%, quindi non facciamo nulla!

                        // MALUS PER LO SPORCO
                        int dirtPenalty = dirts.size();
                        int finalReward = baseReward - dirtPenalty;

                        // Assicuriamoci che paghi almeno 1 moneta
                        if (finalReward < 1) finalReward = 1;

                        score += finalReward;
                        customersServed++;

                        std::cout << "Ordine Consegnato! +" << finalReward << " Monete.";
                        if (goldBoostTimer > 0.0f) std::cout << " (Bonus Bardo x2!)";
                        std::cout << " | Incasso Totale: " << score << std::endl;

                        // --- ATTIVAZIONE BONUS VELOCITÀ ---
                        if (customersServed > 0 && customersServed % 6 == 0) {
                            player.activateSpeedBoost(6.0f); // Usa la funzione del giocatore!
                            std::cout << "\n⚡ COMBO! Hai servito 6 piatti! VELOCITA' AUMENTATA! ⚡\n" << std::endl;
                        }

                        // --- LOGICA LEVEL UP ---
                        if (customersServed > 0 && customersServed % 5 == 0) {
                            currentLevel++;
                            angryCustomers = 0; // azzeramento clienti arrabbiati
                            std::cout << "\n====================================" << std::endl;
                            std::cout << "*** LEVEL UP! LIVELLO " << currentLevel << " ***" << std::endl;
                            if (currentSpawnRate > 2.0f) currentSpawnRate -= 2.0f;
                            if (currentMaxPatience > 15.0f) currentMaxPatience -= 5.0f;
                        }

                        // --- SPAWN SPORCO ---
                        if (rand() % 100 < 40) {
                            dirts.push_back(Dirt(custPos));
                            printDashboard();
                        }

                        player.setHeldItem(-1); // Svuotiamo le mani tramite il Setter!
                        customers.erase(customers.begin() + i); // Il cliente se ne va
                        delivered = true;
                        break;
                    }
                }
            }

            //    // CASO 3: Buttare il cibo (Se premiamo Spazio lontano dai clienti)
            //    if (!delivered) {
            //        std::cout << "Piatto buttato!" << std::endl;
            //        heldItem = -1; // Svuota le mani
            //    }

        }
    }
    // Aggiorniamo lo stato del tasto per il prossimo frame
    spacebarPressed = spaceIsPressed;

    // BUTTARE IL CIBO NEL CESTINO (TASTO C)
    if (cIsPressed && !cWasPressed) {
        if (player.getHeldItem() != -1) { // Se abbiamo qualcosa in mano

            // Controlliamo se siamo vicini al cestino!
            if (glm::distance(player.getPos(), trashCan.getPos()) < 80.0f) {
                std::cout << "Piatto buttato nel cestino!" << std::endl;
                player.setHeldItem(-1); // Svuota le mani
            }
            else {
                std::cout << "Devi avvicinarti al cestino per buttare il cibo!" << std::endl;
            }

        }
    }
    cWasPressed = cIsPressed;

    // --- AZIONE DI PULIZIA (TASTO E) ---
    bool eIsPressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    if (eIsPressed && !eWasPressed) {
        bool cleanedSomething = false;

        for (int i = 0; i < dirts.size(); i++) {
            // Se siamo vicini a una macchia, la puliamo
            if (glm::distance(player.getPos(), dirts[i].getPos()) < 60.0f) {
                dirts.erase(dirts.begin() + i);
                std::cout << "Hai spazzato via una macchia!" << std::endl;
                printDashboard(); // Aggiorna la barra in console
                cleanedSomething = true;
                break; // Puliamo una macchia alla volta
            }
        }

        if (!cleanedSomething) {
            std::cout << "Niente da pulire qui vicino." << std::endl;
        }
    }
    eWasPressed = eIsPressed;

    if (wizard.isWaitingForAnswer()) {

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
            // Costa 15 monete, controlliamo se siamo abbastanza ricchi!
            if (score >= 15) {
                score -= 15;
                dirts.clear(); // MAGIA! 
                wizard.answerPrompt(true);
                std::cout << "Monete rimanenti: " << score << std::endl;
                printDashboard(); // Stampa la barra pulita in console
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}