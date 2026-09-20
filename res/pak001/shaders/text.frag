#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D u_texture;
uniform vec3 u_color = vec3(1.0, 1.0, 1.0);

void main() {
    float sampled = texture(u_texture, TexCoords).r;

    FragColor = vec4(u_color, sampled);
}
