#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Le nostre due nuove variabili per l'animazione!
uniform vec2 texScale;
uniform vec2 texOffset;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	
	// Moltiplichiamo le coordinate per la scala (es. 0.5) e aggiungiamo lo spostamento (es. 0.0 o 0.5)
	TexCoord = vec2(aTexCoord.x * texScale.x + texOffset.x, aTexCoord.y * texScale.y + texOffset.y);
}