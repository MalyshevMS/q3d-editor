#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

uniform mat4 u_model;
uniform mat4 u_viewProjection;
uniform mat3 u_normalMatrix; // transpose(inverse(mat3(u_model)))

out vec2 vUV;
out vec3 vNormal;
out vec3 vFragPos;

void main() {
    vec4 worldPos = u_model * vec4(aPosition, 1.0);
    vFragPos = vec3(worldPos);

    vNormal = u_normalMatrix * aNormal;

    vUV = aUV;

    gl_Position = u_viewProjection * worldPos;
}
