#version 330 core
in vec3 vWorldPos;
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uEmissive;

uniform vec3 uAmbientColor;
uniform int uNumPointLights;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float range;
    float falloff;
};

uniform PointLight uPointLights[50];


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

void main() {
    vec4 texColor = texture(uTexture, vUV);
    vec3 color = texColor.rgb;

    // Ambient + emissive
    vec3 result = color * uAmbientColor + uEmissive;

    // Point lights (distance-only attenuation)
    for(int i = 0; i < uNumPointLights; ++i) {
        vec3 lightPos = uPointLights[i].position;
        vec3 lightColor = uPointLights[i].color;

        float distance = length(lightPos - vWorldPos);
        float attenuation = attenuate(distance, uPointLights[i].range, uPointLights[i].intensity, uPointLights[i].falloff);

        // Add contribution without diffuse angle
        result += color * lightColor * attenuation * 0.2;
    }

    FragColor = vec4(result, texColor.a);

}
