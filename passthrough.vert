#version 330 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv_in;

out vec2 TexCoord;
uniform mat4 uProjection;

void main() {
    TexCoord = uv_in;
    gl_Position = vec4(pos, 0.0, 1.0);
}
