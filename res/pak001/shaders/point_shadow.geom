#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 u_lightSpaceMatrices[6];
uniform int u_lightIndex;

out vec4 vFragPos;

void main() {
    int baseLayer = u_lightIndex * 6;

    for (int face = 0; face < 6; face++) {
        gl_Layer = baseLayer + face;

        for (int i = 0; i < 3; i++) {
            vFragPos = gl_in[i].gl_Position;
            gl_Position = u_lightSpaceMatrices[face] * vFragPos;
            EmitVertex();
        }

        EndPrimitive();
    }
}
