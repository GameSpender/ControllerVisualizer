#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent; // glTF tangent

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
out mat3 vTBN; // TBN matrix

uniform mat4 uMVP;
uniform mat4 uModel; // for world-space calculations

void main()
{
    vec4 worldPos = uModel * vec4(inPos, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform normal to world space
    vec3 N = normalize(mat3(transpose(inverse(uModel))) * inNormal);
    vNormal = N;

    // Tangent and bitangent in world space
    vec3 T = normalize(mat3(uModel) * inTangent.xyz);
    vec3 B = cross(N, T) * inTangent.w;

    vTBN = mat3(T, B, N);

    vUV = inUV;

    gl_Position = uMVP * vec4(inPos, 1.0);
}
