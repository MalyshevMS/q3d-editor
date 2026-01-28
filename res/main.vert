#version 330 core

layout (location = 0) in vec3 input_position;
layout (location = 1) in vec3 input_color;

uniform float u_scale;

out vec3 color;

void main() {
    gl_Position = vec4(input_position.x * u_scale, input_position.y, input_position.z, 1.0);
    color = input_color;
}