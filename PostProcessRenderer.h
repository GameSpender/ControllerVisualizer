#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

class PostProcessRenderer
{
public:
    GLuint shader;      // post-process shader (e.g. gravity ripple)
    GLuint quadVAO;
	GLuint quadVBO;

    PostProcessRenderer(GLuint shaderProgram)
        : shader(shaderProgram)
    {
        initFullscreenQuad();
    }

    ~PostProcessRenderer()
    {
        glDeleteVertexArrays(1, &quadVAO);
    }

    virtual void passUniforms() {}

    void Render(GLuint framebufferTexture)
    {
        glUseProgram(shader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebufferTexture);

        passUniforms();

        //glUniform1i(glGetUniformLocation(shader, "uScene"), 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

private:
    void initFullscreenQuad()
    {
        float quadVerts[] = {
            // pos      // uv
            -1.f, -1.f, 0.f, 0.f,
             1.f, -1.f, 1.f, 0.f,
            -1.f,  1.f, 0.f, 1.f,
             1.f,  1.f, 1.f, 1.f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

        // pos (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // uv_in (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }
};