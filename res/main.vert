#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

uniform float u_scale;

out vec3 vColor;
out vec2 vUV;

void main() {
    gl_Position = vec4(aPosition.x * u_scale, aPosition.y, aPosition.z, 1.0);
    vColor = aColor;
    vUV = aUV;
}