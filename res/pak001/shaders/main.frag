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
    float constant;

    vec3 ambient;
    float linear;

    vec3 diffuse;
    float quadratic;

    vec3 specular;
};

struct SpotLight {
    vec3 position;
    float cutOff;

    vec3 direction;
    float outerCutOff;

    vec3 ambient;
    float constant;

    vec3 diffuse;
    float linear;

    vec3 specular;
    float quadratic;
};

// Uniforms

uniform sampler2D u_texture;
uniform sampler2DArray u_shadowMap;
uniform samplerCubeArray u_cubeMap;
uniform float u_farPlane = 100.f;

uniform float u_bias1 = 0.0005;
uniform float u_bias2 = 0.00005;

uniform Material u_material;
uniform vec3 u_viewPos;

// SSBO

layout(std430, binding = 0) readonly buffer DirLightBuffer {
    DirLight dirLights[];
};

layout(std430, binding = 1) readonly buffer SpotLightBuffer {
    SpotLight spotLights[];
};

layout(std430, binding = 2) readonly buffer PointLightBuffer {
    PointLight pointLights[];
};


layout(std430, binding = 3) readonly buffer LightSpaceMatrices {
    mat4 lightSpaceMatrices[];
};

float calcShadow(vec3 fragPos, vec3 normal, vec3 lightDir, int index) {
    vec4 fragPosLightSpace = lightSpaceMatrices[index] * vec4(fragPos, 1.0);

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    float closestDepth = texture(u_shadowMap, vec3(projCoords.xy, index)).r;
    float currentDepth = projCoords.z;

    float bias = max(
        u_bias1 * (1.0 - dot(normal, lightDir)),
        u_bias2
    );

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

float calcPointShadow(vec3 fragPos, vec3 lightPos, vec3 normal, int index) {
    vec3 fragToLight = fragPos - lightPos;

    float closestDepth = texture(u_cubeMap, vec4(fragToLight, index)).r;

    closestDepth *= u_farPlane;

    float currentDepth = length(fragToLight);

    vec3 lightDir = normalize(lightPos - fragPos);

    float bias = max(
        u_bias1 * (1.0 - dot(normal, lightDir)),
        u_bias2
    );

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texColor, float shadow) {
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

    return ambient + (diffuse + specular) * (1.0 - shadow);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor, float shadow) {
    vec3 lightDir = normalize(light.position - fragPos);

    // Intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (
        light.constant + light.linear * distance + light.quadratic * (distance * distance)
    );

    // Ambient
    vec3 ambient = light.ambient * u_material.ambient * texColor;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * u_material.diffuse) * texColor;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_material.shininess);
    vec3 specular = light.specular * (spec * u_material.specular);

    return ambient + (diffuse + specular) * attenuation * intensity * (1.0 - shadow);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texColor, float shadow) {
    vec3 lightDir = normalize(light.position - fragPos);

    // Ambient
    vec3 ambient = light.ambient * u_material.ambient * texColor;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * u_material.diffuse) * texColor;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_material.shininess);
    vec3 specular = light.specular * (spec * u_material.specular);

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (
        light.constant + light.linear * distance + light.quadratic * distance * distance
    );

    return ambient + (diffuse + specular) * attenuation * (1.0 - shadow);
}

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(u_viewPos - vFragPos);
    vec3 texColor = texture(u_texture, vUV).rgb;

    vec3 result = vec3(0.0);

    int index = 0;

    // Directional
    for (int i = 0; i < dirLights.length(); i++) {
        float shadow = calcShadow(vFragPos, norm, normalize(-dirLights[i].direction), index);

        result += calcDirLight(dirLights[i], norm, viewDir, texColor, shadow);
        index++;
    }

    // Spot
    for (int i = 0; i < spotLights.length(); i++) {
        float shadow = calcShadow(vFragPos, norm, normalize(spotLights[i].position - vFragPos), index);

        result += calcSpotLight(spotLights[i], norm, vFragPos, viewDir, texColor, shadow);

        index++;
    }

    // Point
    for (int i = 0; i < pointLights.length(); i++) {
        float shadow = calcPointShadow(vFragPos, pointLights[i].position, norm, i);

        result += calcPointLight(pointLights[i], norm, vFragPos, viewDir, texColor, shadow);
    }

    FragColor = vec4(result, 1.0);
}
