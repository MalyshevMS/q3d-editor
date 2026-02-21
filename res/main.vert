#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;

uniform mat4 u_mvp;

out vec2 vUV;

void main() {
    gl_Position = u_mvp * vec4(aPosition, 1.0);
    vUV = aUV;
}