#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_texture;

void main() {
    vec3 col = texture(u_texture, TexCoords).rgb;

    FragColor = vec4(1.0 - col, 1.0);
}
