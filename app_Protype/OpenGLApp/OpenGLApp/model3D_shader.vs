#version 330 core

// --- INPUT DAI BUFFER (devono corrispondere a setupMesh() in Mesh.h) ---
layout (location = 0) in vec3 aPos;      // Posizione vertice
layout (location = 1) in vec3 aNormal;   // Normale del vertice
layout (location = 2) in vec2 aTexCoord; // Coordinate UV

// --- OUTPUT verso il Fragment Shader ---
out vec2 TexCoord;
out vec3 FragPos;   // Posizione del frammento nel world space (per la luce)
out vec3 Normal;    // Normale nel world space (per la luce)

// --- MATRICI DI TRASFORMAZIONE ---
uniform mat4 model;      // Trasformazione del singolo oggetto (posizione, scala, rotazione)
uniform mat4 view;       // Trasformazione della camera
uniform mat4 projection; // Proiezione (ortografica o prospettica)

void main()
{
    // Posizione finale sullo schermo
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // -> prende il tavolo di Blender, lo sposta dove vuoi tu (Model), lo guarda dalla telecamera (View) 
    //    e lo schiaccia sul tuo schermo 2D (Projection).

    // Passiamo la posizione world-space al fragment shader per il calcolo luci
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Trasformiamo la normale in world space
    // La matrice inversa-trasposta gestisce correttamente le normali anche con scale non uniformi
    Normal = mat3(transpose(inverse(model))) * aNormal;
    // -> Importare un modello 3D e "schiacciarlo" distorce le sue normali. Usare la matrice inversa-trasposta corregge la deformazione
    //    assicurando che la luce colpisca l'oggetto in modo realistico
    
    
    // Le coordinate UV passano invariate
    TexCoord = aTexCoord;
}
