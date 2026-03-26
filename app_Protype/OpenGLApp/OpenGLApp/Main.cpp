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

#include "Game.h"

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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
const float CAM_HEIGHT = 500.0f;
const float CAM_TILT_DEG = 30.0f;

const glm::vec3 TAVERN_LIGHT_DIR = glm::vec3(-0.3f, -1.0f, -0.5f); // Alto-sinistra
const glm::vec3 TAVERN_LIGHT_COLOR = glm::vec3(1.0f, 0.95f, 0.80f); // Bianco caldo
const float TAVERN_AMBIENT = 0.55f; // Luce ambientale (0=buio pesto, 1=nessuna ombra)

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
//bool firstMouse = true;

Game Tavern(SCR_WIDTH, SCR_HEIGHT);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


void drawTavernModel(
    Shader& shader, Model& model,
    float screenX, float screenY,
    float scaleX, float scaleY, float scaleZ,
    float rotX, float rotY = 0.0f) // Sostituiamo la rotazione Y con l'inclinazione X!
{
    glm::mat4 mat = glm::mat4(1.0f);

    // 1. Z-SORTING: Usiamo la Y per calcolare la Z! 
    // In questo modo i personaggi cammineranno correttamente DAVANTI e DIETRO i tavoli.
    mat = glm::translate(mat, glm::vec3(screenX, screenY, (SCR_HEIGHT - screenY)));
    mat = glm::rotate(mat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    // 2. INCLINAZIONE 2.5D: Ribaltiamo il modello verso la telecamera per mostrare le gambe
    mat = glm::rotate(mat, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));

    // 3. SCALA:
    mat = glm::scale(mat, glm::vec3(scaleX, scaleY, scaleZ));

    shader.setMat4("model", mat);
    model.Draw(shader);
}



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

    Shader modelShader("model3D_shader.vs", "model3D_shader.fs");


    Model tableModel("resources/models/table/table.obj");
    Model counterModel("resources/models/counter/counter.obj");

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


    Tavern.Init();

    unsigned int texFloor = loadTexture("resources/textures/wood.jpeg");

    unsigned int texBeer = loadTexture("resources/textures/beer.png");
    unsigned int texMeat = loadTexture("resources/textures/meat.png");
    unsigned int texBread = loadTexture("resources/textures/bread.png");
    unsigned int texSoup = loadTexture("resources/textures/soup.png");


    unsigned int foodTextures[4];
    foodTextures[0] = texBeer;   // 0 = Birra
    foodTextures[1] = texMeat;   // 1 = Carne
    foodTextures[2] = texSoup; // 2 = Zuppa (tappabuchi temporaneo)
    foodTextures[3] = texBread;  // 3 = Pane (tappabuchi temporaneo)



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

    unsigned int texCustDown = loadTexture("resources/textures/customer_front.png");
    unsigned int texCustUp = loadTexture("resources/textures/customer_back.png");
    unsigned int texCustLeft = loadTexture("resources/textures/customer_left.png");
    unsigned int texCustRight = loadTexture("resources/textures/customer_right.png");
    unsigned int texCustSeated = loadTexture("resources/textures/customer1.png");

    unsigned int texTrash = loadTexture("resources/textures/trashcan.png");

    // 0=Giu, 1=Su, 2=Sinistra, 3=Destra
    unsigned int custTextures[4] = { texCustDown, texCustUp, texCustLeft, texCustRight };


    Tavern.cubeVAO = cubeVAO;
    Tavern.texTrash = texTrash;
    Tavern.texCustSeated = texCustSeated;
    Tavern.texWizUp = texWizUp; Tavern.texWizDown = texWizDown;
    Tavern.texBardUp = texBardUp; Tavern.texBardDown = texBardDown; Tavern.texBardPlay = texBardPlay;

    for (int i = 0; i < 4; i++) {
        Tavern.foodTextures[i] = foodTextures[i];
        Tavern.playerTextures[i] = playerTextures[i];
        Tavern.custTextures[i] = custTextures[i];
    }


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Tavern.ProcessInput(window, deltaTime);
        Tavern.Update(deltaTime);

        // ============ RENDERING ============

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // ==========================================
        // TELECAMERA 2D (Per oste, clienti e UI)
        // TELECAMERA 3D (Per tavoli, bancone)
        // ==========================================

        glm::mat4 sharedProjection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1000.0f, 1000.0f);
        glm::mat4 sharedView = glm::mat4(1.0f);

        ourShader.use();
        ourShader.setMat4("view", sharedView);
        ourShader.setMat4("projection", sharedProjection);

        modelShader.use();
        modelShader.setMat4("view", sharedView);
        modelShader.setMat4("projection", sharedProjection);
        modelShader.setVec3("lightDir", TAVERN_LIGHT_DIR);
        modelShader.setVec3("lightColor", TAVERN_LIGHT_COLOR);
        modelShader.setFloat("ambientStrength", TAVERN_AMBIENT);

        Tavern.Render(ourShader, modelShader, tableModel, counterModel);


        // ---> IMPORTANTE: Valori predefiniti per TUTTI gli oggetti non animati! <---
        ourShader.setVec2("texScale", 1.0f, 1.0f);
        ourShader.setVec2("texOffset", 0.0f, 0.0f);

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


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

