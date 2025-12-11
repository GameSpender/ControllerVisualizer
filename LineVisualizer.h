#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

class LineVisualizer {
public:
    glm::vec2 start;
    glm::vec2 end;
    glm::vec3 color;

    LineVisualizer() = default;
    LineVisualizer(glm::vec2 s, glm::vec2 e, glm::vec3 c);
    ~LineVisualizer();

    void Draw(GLuint shader, int screenW, int screenH);

private:
    GLuint VAO = 0, VBO = 0;
    void setup();
};



LineVisualizer::LineVisualizer(glm::vec2 start, glm::vec2 end, glm::vec3 color)
    : start(start), end(end), color(color)
{
    setup();
}

LineVisualizer::~LineVisualizer()
{
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

void LineVisualizer::setup()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // space for 2 vertices: (vec2 pos + vec4 col) * 2
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, nullptr, GL_DYNAMIC_DRAW);

    // pos: location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);

    // color: location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 2));

    glBindVertexArray(0);
}

void LineVisualizer::Draw(GLuint shader, int screenW, int screenH)
{
    // Convert world coords → NDC
    auto toNDC = [&](glm::vec2 p) {
        float x = (p.x / (float)screenW) * 2.0f - 1.0f;
        float y = (p.y / (float)screenH) * 2.0f - 1.0f;
        return glm::vec2(x, y);
        };

    glm::vec2 p0 = toNDC(start);
    glm::vec2 p1 = toNDC(end);

    float col4[4] = { color.r, color.g, color.b, 1.0f };

    float verts[12] = {
        p0.x, p0.y,  col4[0], col4[1], col4[2], col4[3],
        p1.x, p1.y,  col4[0], col4[1], col4[2], col4[3]
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glUseProgram(shader);

    // shader requires these:
    glUniform2f(glGetUniformLocation(shader, "uPos"), 0.0f, 0.0f);
    glUniform1f(glGetUniformLocation(shader, "uB"), 0.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}