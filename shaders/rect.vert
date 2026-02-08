#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;
out vec3 vWorldPos;
out vec3 vNormal;

uniform mat4 uModel;
uniform mat4 uMVP;

void main()
{
    vec4 worldPos = uModel * vec4(aPos.xy, 0.0, 1.0);

    vWorldPos = worldPos.xyz;

    // Sprite normal (facing forward in local Z)
    vNormal = normalize(mat3(uModel) * vec3(0.0, 0.0, 1.0));

    vUV = aUV;

    gl_Position = uMVP * vec4(aPos.xy, 0.0, 1.0);
}
