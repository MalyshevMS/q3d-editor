#version 330 core
layout (location = 0) in vec4 aPosTexCoords;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPosTexCoords.x, aPosTexCoords.y, 0.0, 1.0); 
    TexCoords = aPosTexCoords.zw;
}
