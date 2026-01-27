#version 330 core

in vec3 vColor;
out vec4 vFragment;

void main() {
    vFragment = vec4(vColor, 1.0);
}