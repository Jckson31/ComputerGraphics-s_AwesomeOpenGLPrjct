#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

// =========================================================
// STRUTTURA VERTICE
// Ogni vertice del modello 3D contiene queste informazioni.
// Assimp le legge dal file .obj/.fbx e noi le salviamo qui.
// =========================================================
struct Vertex {
    glm::vec3 Position;   // Posizione XYZ nello spazio 3D
    glm::vec3 Normal;     // Normale (per il calcolo della luce)
    glm::vec2 TexCoords;  // Coordinate UV della texture
    glm::vec3 Tangent;    // Usato per le normal map (opzionale)
    glm::vec3 Bitangent;  // Usato per le normal map (opzionale)
};

// =========================================================
// STRUTTURA TEXTURE
// Ogni texture caricata da Assimp viene salvata qui.
// =========================================================
struct Texture {
    unsigned int id;   // ID OpenGL della texture (generato da glGenTextures)
    std::string type;  // Tipo: "texture_diffuse", "texture_specular", ecc.
    std::string path;  // Percorso file (usato per evitare di caricarla due volte)
};

// =========================================================
// CLASSE MESH
// Una Mesh è un singolo oggetto geometrico (es. il piano
// del tavolo, una gamba, una panca).
// Un modello Blender può essere composto da più Mesh.
// =========================================================
class Mesh {
public:
    // --- DATI DELLA MESH ---
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    unsigned int VAO; // Vertex Array Object (contiene tutto il setup OpenGL)

    // --- COSTRUTTORE ---
    // Riceve i dati da Model.h (che li ha letti da Assimp) e li carica sulla GPU
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh(); // Carica tutto sulla GPU
    }

    // --- DISEGNA LA MESH ---
    // Viene chiamata da Model::Draw() per ogni mesh del modello.
    // Lo shader passato deve essere quello 3D (model_shader).
    void Draw(class Shader& shader)
    {
        // Contiamo quante texture diffuse e specular abbiamo
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // Attiva lo slot texture corretto

            // Costruiamo il nome dell'uniform: es. "texture_diffuse1"
            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);

            // Passa la texture allo shader
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // --- DISEGNA LA GEOMETRIA ---
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Torna allo slot 0 per sicurezza
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // Buffer Objects interni
    unsigned int VBO, EBO;

    // --- SETUP SULLA GPU ---
    // Crea VAO, VBO, EBO e configura i puntatori agli attributi.
    // Viene chiamata una sola volta nel costruttore.
    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Carica i vertici nel VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // Carica gli indici nel EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // --- CONFIGURAZIONE ATTRIBUTI ---
        // Posizione (location = 0): 3 float, offset 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Normale (location = 1): 3 float, offset dopo Position
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // Coordinate UV (location = 2): 2 float, offset dopo Normal
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        // Tangent (location = 3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

        // Bitangent (location = 4)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0); // Sbinda il VAO: configurazione completata
    }
};