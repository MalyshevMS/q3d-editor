#version 430 core

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

// Structs

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Uniforms

uniform sampler2D u_texture;
uniform Material u_material;
uniform vec3 u_viewPos;

// SSBO

layout(std430, binding = 0) readonly buffer DirLightBuffer {
    DirLight dirLights[];
};

layout(std430, binding = 1) readonly buffer PointLightBuffer {
    PointLight pointLights[];
};

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor) {
    vec3 lightDir = normalize(-light.direction);

    // Ambient
    vec3 ambient = light.ambient * u_material.ambient * texColor;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * u_material.diffuse) * texColor;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_material.shininess);
    vec3 specular = light.specular * (spec * u_material.specular);

    return (ambient + diffuse + specular);
}

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(u_viewPos - vFragPos);
    vec3 texColor = texture(u_texture, vUV).rgb;

    vec3 result = vec3(0.0);

    for (int i = 0; i < dirLights.length(); i++) {
        result += calcDirLight(dirlights[i], norm, viewDir, texColor);
    }

    for (int i = 0; i < pointLights.length(); i++) {
        // result += calcPointLight(pointLights[i]);
    }

    FragColor = vec4(result, 1.0);
}
