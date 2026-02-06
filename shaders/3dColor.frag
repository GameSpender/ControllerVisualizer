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
uniform vec3 uCameraPos;
uniform vec3 uAmbientColor;

const float PI = 3.14159265;

// -------------------------
// Point lights
// -------------------------
struct PointLight {
    vec3 position;
    vec3 color;

    float intensity;
    float range;
    float falloff;
};

uniform int uNumPointLights;
uniform PointLight uPointLights[50]; // max 50 lights for now

// -------------------------
// PBR helper
// -------------------------
vec3 calculatePBR(vec3 albedo, float metallic, float roughness,
                  vec3 N, vec3 V, vec3 L)
{
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);

    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    float D = a2 / (PI * denom * denom);

    float k = a / 2.0;
    float Gv = NdotV / (NdotV * (1.0 - k) + k);
    float Gl = NdotL / (NdotL * (1.0 - k) + k);
    float G = Gv * Gl;

    vec3 spec = (D * G * F) / (4.0 * NdotL * NdotV + 0.001);
    vec3 diff = (1.0 - metallic) * albedo / PI;

    return (diff + spec) * NdotL;
}


float sqr(float x) {
    return x * x;
}

float attenuate(float distance, float range, float intensity, float falloff){
    float s = distance / range;
    if (s >= 1.0)
        return 0.0;
    float s2 = sqr(s);

    return intensity * sqr(1 - s2) / (1 + falloff * s2);
}

float attenuate_no_cusp(float distance, float range, float intensity, float falloff){
    float s = distance / range;
    if (s >= 1.0)
        return 0.0;
    float s2 = sqr(s);

    return intensity * sqr(1 - s2) / (1 + falloff * s);
}

vec3 calculatePointLight(vec3 albedo, float metallic, float roughness, vec3 N, vec3 V, PointLight light, vec3 fragPos)
{
    float distance = length(light.position - fragPos);
    
    vec3 L = normalize(light.position - fragPos);


    float attenuation = attenuate_no_cusp(distance, light.range, light.intensity, light.falloff);

    vec3 radiance = light.color * attenuation;

    vec3 color = calculatePBR(albedo, metallic, roughness, N, V, L);
    return color * radiance;
}

// -------------------------
// Main
// -------------------------
void main()
{
    vec3 albedo = (uHasBaseColorTex == 1 ? texture(uBaseColorTex, vUV).rgb : vec3(1.0)) * uBaseColorFactor.rgb;
    float metallic = uMetallicFactor;
    float roughness = clamp(uRoughnessFactor, 0.001, 1.0);
    vec3 emissive = uEmissiveFactor;
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);

    // Ambient + emissive
    vec3 color = albedo * uAmbientColor + emissive;

    // Point lights
    for (int i = 0; i < uNumPointLights; ++i)
        color += calculatePointLight(albedo, metallic, roughness, N, V, uPointLights[i], vWorldPos);

    outCol = vec4(color, uBaseColorFactor.a);
}
