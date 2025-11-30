#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for translate/rotate/scale
#include <glm/gtc/type_ptr.hpp>

class SpriteRenderer {
public:
    GLuint shader;
    GLuint quadVAO;

    SpriteRenderer(GLuint shaderProgram) : shader(shaderProgram) {
        initRenderData();
    }

    ~SpriteRenderer() {
        glDeleteVertexArrays(1, &quadVAO);
    }

    // -------------------------------
// 1. Draw using mat3 transform
// -------------------------------
    void Draw(GLuint texture, const glm::mat3& transform) {
        glm::mat4 model(1.0f);

        // Copy rotation+scale components
        model[0][0] = transform[0][0]; // a
        model[0][1] = transform[0][1]; // c
        model[1][0] = transform[1][0]; // b
        model[1][1] = transform[1][1]; // d

        // Translation
        model[3][0] = transform[2].x;
        model[3][1] = transform[2].y;

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModel"), 1, GL_FALSE, glm::value_ptr(model));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }


    void Draw(GLuint texture, glm::vec2 pos, glm::vec2 size, float rotation = 0.0f) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(pos, 0.0f));
        model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); // center pivot
        model = glm::rotate(model, rotation, glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModel"), 1, GL_FALSE, glm::value_ptr(model));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

private:
    void initRenderData() {

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // quad vertices: pos (x,y), tex coords (s,t)
        float vertices[] = {
            // x   y   s   t
            -0.5f, -0.5f, 0.0f, 0.0f,
             0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 1.0f,

            -0.5f, -0.5f, 0.0f, 0.0f,
             0.5f, -0.5f, 1.0f, 0.0f,
             0.5f,  0.5f, 1.0f, 1.0f
        };


        GLuint VBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glDeleteBuffers(1, &VBO); // VAO keeps the reference
    }
};
