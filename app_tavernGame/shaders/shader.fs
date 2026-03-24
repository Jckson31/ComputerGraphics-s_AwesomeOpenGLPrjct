#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // Le coordinate dell'immagine

uniform sampler2D texture1; // La nostra immagine
uniform vec3 noteColor;     // Il colore (lo useremo come "filtro")

void main()
{
    // Leggiamo il colore del pixel dall'immagine
    vec4 texColor = texture(texture1, TexCoord);
    
    // TRUCCO PER LA TRASPARENZA:
    // Se il pixel è quasi completamente trasparente (Alpha < 0.1), scartalo e non disegnarlo!
    if(texColor.a < 0.1)
        discard;
        
    // Moltiplichiamo l'immagine per il colore. 
    // Se noteColor è bianco (1,1,1), l'immagine mantiene i suoi colori originali.
    // Se noteColor è rosso (1,0,0), l'immagine diventerà rossastra (utile per i clienti arrabbiati!).
    FragColor = texColor * vec4(noteColor, 1.0);
}