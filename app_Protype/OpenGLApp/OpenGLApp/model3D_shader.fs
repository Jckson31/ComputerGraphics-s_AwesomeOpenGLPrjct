#version 330 core
 
out vec4 FragColor;
 
// --- INPUT dal Vertex Shader ---
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
 
// --- TEXTURE DIFFUSE DEL MODELLO ---
// Assimp assegna la texture con il nome "texture_diffuse1"
uniform sampler2D texture_diffuse1;
 
// --- CONTROLLO: ha una texture o usiamo il colore piatto?
// true  = usa la texture PNG del modello
// false = usa flatColor (utile se il modello non ha texture)
uniform bool hasDiffuseTexture;
uniform vec3 flatColor; // Colore di fallback (es. marrone legno)
 
// --- LUCE DIREZIONALE ---
// Per un gioco top-down 2.5D vogliamo una luce semplice dall'alto/obliqua
// che dia volume ai modelli senza sembrare un rendering fotorealistico.
uniform vec3 lightDir;       // Direzione della luce (normalizzata in C++)
uniform vec3 lightColor;     // Colore della luce (es. bianco caldo: 1.0, 0.95, 0.85)
uniform float ambientStrength; // Luce ambientale minima (es. 0.5 per non avere ombre nere)
 
void main()
{
    // --- COLORE BASE ---
    vec4 baseColor;
    if (hasDiffuseTexture) {  
        baseColor = texture(texture_diffuse1, TexCoord);
        // Trasparenza: se il pixel è trasparente, scartalo
        if (baseColor.a < 0.1)  
            discard;
    } else {
        baseColor = vec4(flatColor, 1.0);  // Usa il colore "piatto" se non c'è una texture
    }
 
    // --- CALCOLO LUCE DIFFUSA (Lambertiana) ---
    vec3 norm = normalize(Normal);
    vec3 lightDirN = normalize(-lightDir); // La luce arriva DA questa direzione

 
    // dot product: quanto la superficie è "girata verso" la luce?
    // max(..., 0.0) evita valori negativi (luce "dall'altra parte")
    float diff = max(dot(norm, lightDirN), 0.0);  //restituisce 1.0 se la superficie è perfettamente rivolta alla luce, 0.5 se è a 45°, 0.0 se è perpendicolare o rivolta dall'altra parte
                                                  // max toglie i numeri negativi per non creare luce nera
    // --- COMPONENTI LUCE ---
    vec3 ambient  = ambientStrength * lightColor;        // Luce base ovunque
    vec3 diffuse  = diff * lightColor;                   // Luce diretta
 
    // --- COLORE FINALE ---
    // Moltiplichiamo il colore base per la luce calcolata
    vec3 result = (ambient + diffuse) * baseColor.rgb;
    FragColor = vec4(result, baseColor.a);
}
 