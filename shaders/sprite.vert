#version 330 core
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

out vec2 vUV;

uniform mat4 uModel;
uniform mat4 uMVP;

void main() {
    vec4 worldPos = uModel * vec4(inPos, 0.0, 1.0);
    gl_Position = uMVP * vec4(inPos.xy, 0.0, 1.0);
    vUV = inUV;
}
