#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 outCol;

// -------------------------
// Material
// -------------------------
uniform sampler2D uBaseColorTex;
uniform vec4 uBaseColorFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform vec3 uEmissiveFactor;
uniform int uHasBaseColorTex; // 1 = texture present, 0 = no texture

// -------------------------
// Lighting
// -------------------------
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uCameraPos;
uniform vec3 uAmbientColor;

const float PI = 3.14159265;

// -------------------------
// PBR helper (simplified GGX)
// -------------------------
vec3 calculatePBR(vec3 albedo, float metallic, float roughness,
                  vec3 N, vec3 V, vec3 L)
{
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Fresnel
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);

    // GGX Distribution
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    float D = a2 / (PI * denom * denom);

    // Geometry
    float k = a / 2.0;
    float Gv = NdotV / (NdotV * (1.0 - k) + k);
    float Gl = NdotL / (NdotL * (1.0 - k) + k);
    float G = Gv * Gl;

    vec3 spec = (D * G * F) / (4.0 * NdotL * NdotV + 0.001);
    vec3 diff = (1.0 - metallic) * albedo / PI;

    return (diff + spec) * NdotL;
}


void main()
{
    vec3 albedo = uHasBaseColorTex == 1 
                  ? texture(uBaseColorTex, vUV).rgb * uBaseColorFactor.rgb
                  : uBaseColorFactor.rgb; // Use factor only if no texture

    float metallic = uMetallicFactor;
    float roughness = clamp(uRoughnessFactor, 0.04, 1.0);
    vec3 emissive = uEmissiveFactor;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 L = normalize(uLightDir);

    vec3 color = calculatePBR(albedo, metallic, roughness, N, V, L);

    // Apply lighting
    color *= uLightColor;
    color += emissive;
    color += albedo * uAmbientColor;

    outCol = vec4(color, uBaseColorFactor.a);
}
