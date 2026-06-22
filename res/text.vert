#version 330 core
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 u_mvp;

void main() {
    gl_Position = u_mvp * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
