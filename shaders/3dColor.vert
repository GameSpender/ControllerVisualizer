#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

uniform mat4 uMVP;
uniform mat4 uModel;

void main()
{
    vec4 worldPos = uModel * vec4(inPos, 1.0);
    vWorldPos = worldPos.xyz;

    vNormal = normalize(mat3(transpose(inverse(uModel))) * inNormal);
    vUV = inUV;

    gl_Position = uMVP * vec4(inPos, 1.0);
}
