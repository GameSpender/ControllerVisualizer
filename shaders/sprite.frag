#version 330 core
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
uniform vec3 uCameraPos;

void main() {
    vec4 texColor = texture(uTexture, vUV);
    vec3 color = texColor.rgb;

    // Ambient
    vec3 result = color * uAmbientColor;

    // Simple point light diffuse
    for(int i=0; i<uNumPointLights; ++i) {
        vec3 lightDir = normalize(uPointLights[i].position - vec3(0.0)); // assume at origin for quad
        float diff = max(dot(vec3(0,0,1), lightDir), 0.0);
        float attenuation = uPointLights[i].intensity / (1.0 + uPointLights[i].falloff * length(uPointLights[i].position));
        result += color * uPointLights[i].color * diff * attenuation;
    }

    // Add emissive
    result += uEmissive;

    FragColor = vec4(result, texColor.a);
}
