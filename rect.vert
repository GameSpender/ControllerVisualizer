#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inCol;
out vec4 chCol;

uniform float uX;
uniform float uY;
uniform float uS; // skaliranje geometrijskog tela množenjem

void main()
{
    gl_Position = vec4(inPos.x + uX, inPos.y * uS + uY, 0.0, 1.0);
    chCol = vec4(inCol, 1.0);
}