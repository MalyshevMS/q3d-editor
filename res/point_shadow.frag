#version 430 core

in vec4 vFragPos;

uniform vec3 u_lightPos;
uniform float u_farPlane;

void main() {
    float lightDistance = length(vFragPos.xyz - u_lightPos);

    lightDistance = lightDistance / u_farPlane;

    gl_FragDepth = lightDistance;
}
