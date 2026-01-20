#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
in mat3 vTBN;

out vec4 outCol;

// -------------------------
// Material textures
// -------------------------
uniform sampler2D uAlbedoTex;
uniform sampler2D uMetallicRoughnessTex;
uniform sampler2D uNormalTex;
uniform sampler2D uOcclusionTex;
uniform sampler2D uEmissiveTex;

// -------------------------
// Material factors
// -------------------------
uniform vec4 uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform vec3 uEmissive;

// -------------------------
// Texture usage flags
// -------------------------
uniform bool uUseTex;

// -------------------------
// Lighting
// -------------------------
uniform vec3 uLightDir = normalize(vec3(0.5, 1.0, 0.3)); // directional light
uniform vec3 uLightColor = vec3(1.0);
uniform vec3 uCameraPos;

// -------------------------
// Helper: simple PBR diffuse+specular
// -------------------------
vec3 calculatePBR(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 viewDir, vec3 lightDir)
{
    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);
    vec3 L = normalize(lightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // Fresnel (Schlick)
    vec3 F0 = vec3(0.04); // default for dielectrics
    F0 = mix(F0, albedo, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);

    // Distribution GGX
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
    float D = alpha2 / (3.14159265 * denom * denom);

    // Geometry (Schlick-GGX)
    float k = alpha / 2.0;
    float G_Schlick = NdotV / (NdotV * (1.0 - k) + k);
    float G = G_Schlick * (NdotL / (NdotL * (1.0 - k) + k));

    vec3 spec = D * G * F / (4.0 * NdotL * NdotV + 0.001);
    vec3 diff = albedo * (1.0 - metallic) / 3.14159265;

    return (diff + spec) * NdotL;
}

void main()
{
    // -------------------------
    // Sample textures
    // -------------------------

    vec3 color = uBaseColor.rgb;
    
    if(uUseTex){
        vec3 albedo = uBaseColor.rgb;
        albedo *= texture(uAlbedoTex, vUV).rgb;

        float metallic = uMetallic;
        float roughness = uRoughness;
        if (textureSize(uMetallicRoughnessTex, 0).x > 0) {
            vec3 mr = texture(uMetallicRoughnessTex, vUV).rgb;
            metallic = mr.b;      // glTF convention: B = metallic
            roughness = mr.g;     // glTF convention: G = roughness
        }

    
        vec3 normal = normalize(vNormal);

        // Sample normal map
        if (textureSize(uNormalTex, 0).x > 0) {
            vec3 n = texture(uNormalTex, vUV).rgb;
            n = n * 2.0 - 1.0;           // Convert from [0,1] -> [-1,1]
            normal = normalize(vTBN * n); // Transform tangent-space normal to world space
        }

        vec3 emissive = uEmissive;
        if (textureSize(uEmissiveTex, 0).x > 0)
            emissive += texture(uEmissiveTex, vUV).rgb;

        // -------------------------
        // Simple directional lighting
        // -------------------------
        vec3 viewDir = normalize(uCameraPos - vWorldPos);
        color = calculatePBR(albedo, metallic, roughness, normal, viewDir, normalize(uLightDir));

        // Apply emissive
        color += emissive;
    }


    outCol = vec4(color, uBaseColor.a);
}
