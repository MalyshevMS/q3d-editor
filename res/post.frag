#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_texture;

uniform vec2 u_blurVector;
uniform float u_blurForward;
uniform float u_chromaticIntensity = 0.003;
uniform float u_vignettePower = 1.0;

vec3 applyMotionBlur(vec2 uv) {
    vec3 blurColor = vec3(0.0);
    vec2 texCoordOffset = uv;
    vec2 radialDir = uv - vec2(0.5);
    vec2 totalOffset = u_blurVector + (radialDir * u_blurForward);

    const int SAMPLES = 8;
    for (int i = 0; i < SAMPLES; i++) {
        blurColor += texture(u_texture, texCoordOffset).rgb;
        texCoordOffset += totalOffset / float(SAMPLES);
    }
    return blurColor / float(SAMPLES);
}

vec3 applyChromaticAberration(vec2 uv, vec3 baseColor) {
    if (u_chromaticIntensity <= 0.0) return baseColor;

    vec2 dist = uv - vec2(0.5);
    vec2 offset = dist * u_chromaticIntensity;

    float r = texture(u_texture, uv - offset).r;
    float g = baseColor.g;
    float b = texture(u_texture, uv + offset).b;

    return vec3(r, g, b);
}

vec3 applyVignette(vec2 uv, vec3 baseColor) {
    vec2 uvDist = uv - vec2(0.5);

    float vignette = uvDist.x * uvDist.x + uvDist.y * uvDist.y;

    return baseColor * (1.0 - vignette * u_vignettePower);
}

vec3 applyColorCorrection(vec3 baseColor) {
    // contrast invesion
    vec3 contrastColor = (baseColor - 0.5) * 1.1 + 0.5;

    // cherno-belyi
    // float gray = dot(contrastColor, vec3(0.299, 0.587, 0.114));
    // return vec3(gray);

    return contrastColor;
}

void main() {
    vec3 color = applyMotionBlur(TexCoords);

    color = applyChromaticAberration(TexCoords, color);

    color = applyVignette(TexCoords, color);

    color = applyColorCorrection(color);

    FragColor = vec4(color, 1.0);
}
