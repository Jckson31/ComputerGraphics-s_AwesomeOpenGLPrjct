#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // Le coordinate dell'immagine

uniform sampler2D texture1; // La nostra immagine
uniform vec3 noteColor;     // Nel caso dei clienti, sarà il NUOVO colore dei vestiti!

// --- NUOVE VARIABILI PER IL PALETTE SWAP ---
uniform bool usePaletteSwap;       // INTERRUTTORE: VERO se stiamo disegnando un cliente, FALSO per gli altri oggetti
uniform vec3 originalClothesColor; // Il colore esatto della maglietta nel tuo disegno originale
uniform vec3 originalHairColor;    // Il colore esatto dei capelli nel tuo disegno originale
uniform vec3 newHairColor;         // Il NUOVO colore che vogliamo dare ai capelli del cliente

void main()
{
    // 1. Leggiamo il colore base del pixel dal tuo disegno (texture1)
    vec4 texColor = texture(texture1, TexCoord);
    
    // 2. TRUCCO PER LA TRASPARENZA (Invariato)
    if(texColor.a < 0.1)
        discard;
        
    // 3. Controlliamo l'interruttore: è un cliente da ricolorare?
    if (usePaletteSwap) {
        
        // La funzione "distance" calcola la differenza matematica tra due colori.
        // Se la distanza è piccolissima (es. minore di 0.15), significa che i colori sono uguali!
        float diffClothes = distance(texColor.rgb, originalClothesColor);
        float diffHair = distance(texColor.rgb, originalHairColor);
        
        // Se il pixel ha lo stesso colore della maglietta originale...
        if (diffClothes < 0.15) {
            // ...sostituiscilo con 'noteColor' (il colore casuale che decideremo in C++)
            FragColor = vec4(noteColor, texColor.a);
        }
        // Altrimenti, se il pixel ha lo stesso colore dei capelli originali...
        else if (diffHair < 0.15) {
            // ...sostituiscilo con 'newHairColor'
            FragColor = vec4(newHairColor, texColor.a);
        }
        // Se non è né maglietta né capelli (è pelle, scarpe, occhi)...
        else {
            // ...disegnalo esattamente coi suoi colori originali!
            FragColor = texColor;
        }
    }
    else {
        // 4. Se NON è un cliente (è un tavolo, il cibo o il Mago)
        // Usiamo il tuo vecchio metodo: applichiamo il colore come "lente" su tutta l'immagine.
        FragColor = texColor * vec4(noteColor, 1.0);
    }
}

//version 330 core
//out vec4 FragColor;

//in vec2 TexCoord; // Le coordinate dell'immagine

//uniform sampler2D texture1; // La nostra immagine
//uniform vec3 noteColor;     // Il colore (lo useremo come "filtro")

//void main()
//{
//    // Leggiamo il colore del pixel dall'immagine
//    vec4 texColor = texture(texture1, TexCoord);
    
//    // TRUCCO PER LA TRASPARENZA:
//    // Se il pixel è quasi completamente trasparente (Alpha < 0.1), scartalo e non disegnarlo!
//    if(texColor.a < 0.1)
//        discard;
        
    // Moltiplichiamo l'immagine per il colore. 
    // Se noteColor è bianco (1,1,1), l'immagine mantiene i suoi colori originali.
    // Se noteColor è rosso (1,0,0), l'immagine diventerà rossastra (utile per i clienti arrabbiati!).
//    FragColor = texColor * vec4(noteColor, 1.0);
//}