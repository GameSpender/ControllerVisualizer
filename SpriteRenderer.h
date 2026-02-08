#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for translate/rotate/scale
#include <glm/gtc/type_ptr.hpp>
#include "Services.h"
#include "AssetManager.h"
#include "LightManager.h"
#include "Sprite3D.h"

class SpriteRenderer {
public:
    GLuint shader;
    GLuint quadVAO;
    GLuint quadVBO;

    SpriteRenderer(GLuint shaderProgram) : shader(shaderProgram) {
        initRenderData();
    }

    ~SpriteRenderer() {
        glDeleteVertexArrays(1, &quadVAO);
    }

	void Draw(Texture texture,
        const glm::mat4& model,
        const glm::mat4& view,
        const glm::mat4& proj,
        const glm::vec3& cameraPos) const
    {
        glUseProgram(shader);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // ----------------------------------
        // Lights
        // ----------------------------------
        glm::vec3 ambientColor = glm::vec3(0.01f);
        if (Services::lights) ambientColor = Services::lights->ambientColor;
        glUniform3fv(glGetUniformLocation(shader, "uAmbientColor"), 1, glm::value_ptr(ambientColor));

		auto activeLights = Services::lights->getActiveLights();
        int numPointLights = 0;

        for (int i = 0; i < activeLights.size(); ++i) {
            auto pl = std::dynamic_pointer_cast<PointLight2D>(activeLights[i]);
            if (!pl) continue;
            if (numPointLights >= 50) break;

            std::string prefix = "uPointLights[" + std::to_string(i) + "].";

            glUniform3fv(glGetUniformLocation(shader, (prefix + "position").c_str()), 1, glm::value_ptr(pl->position));
            glUniform3fv(glGetUniformLocation(shader, (prefix + "color").c_str()), 1, glm::value_ptr(pl->color));
            glUniform1f(glGetUniformLocation(shader, (prefix + "intensity").c_str()), pl->intensity);
            glUniform1f(glGetUniformLocation(shader, (prefix + "range").c_str()), pl->range);
            glUniform1f(glGetUniformLocation(shader, (prefix + "falloff").c_str()), pl->falloff);

            numPointLights++;
        }
        glUniform1i(glGetUniformLocation(shader, "uNumPointLights"), numPointLights);
        glUniform3fv(glGetUniformLocation(shader, "uCameraPos"), 1, glm::value_ptr(cameraPos));

        // ----------------------------------
        // Transform
        // ----------------------------------
        glm::mat4 mvp = proj * view * model;
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));

        // ----------------------------------
        // Texture
        // ----------------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

        glUniform3fv(glGetUniformLocation(shader, "uEmissive"), 1, glm::value_ptr(texture.emissive));

        // ----------------------------------
        // Draw quad
        // ----------------------------------
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(0);
    }

    void Draw(std::shared_ptr<Sprite3D> sprite,
        const glm::mat4& view,
        const glm::mat4& proj,
        const glm::vec3& cameraPos) const
    {
        if (!sprite) return;

        auto tex = sprite->texture.lock();
        if (!tex) return;

        glm::mat4 modelMatrix(1.0f);

        glm::vec3 worldPos = glm::vec3(sprite->getWorldMatrix()[3]);

        switch (sprite->mode)
        {
        case Sprite3D::Mode::Normal:
        {
            modelMatrix = sprite->getWorldMatrix();
            break;
        }

        case Sprite3D::Mode::Billboard:
        {
            // Full spherical billboard
            glm::vec3 forward = glm::normalize(cameraPos - worldPos);
            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
            glm::vec3 up = glm::cross(forward, right);

            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(right, 0.0f);
            rot[1] = glm::vec4(up, 0.0f);
            rot[2] = glm::vec4(forward, 0.0f);

            modelMatrix =
                glm::translate(glm::mat4(1.0f), worldPos) *
                rot *
                scale(glm::mat4(1.0f), sprite->scale);
            break;
        }

        case Sprite3D::Mode::BillboardVertical:
        {
            // Y-up cylindrical billboard
            glm::vec3 forward = cameraPos - worldPos;
            forward.y = 0.0f;
            forward = glm::normalize(forward);

            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
            glm::vec3 up = glm::vec3(0, 1, 0);

            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(right, 0.0f);
            rot[1] = glm::vec4(up, 0.0f);
            rot[2] = glm::vec4(forward, 0.0f);

            modelMatrix =
                glm::translate(glm::mat4(1.0f), worldPos) *
                rot *
				scale(glm::mat4(1.0f), sprite->scale)
                ;
            break;
        }
        }

		Texture texRef = *tex.get();
        // Delegate to pure draw
        Draw(texRef, modelMatrix, view, proj, cameraPos);
    }


private:
    void initRenderData() {

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // quad vertices: pos (x,y), tex coords (s,t)
        float vertices[] = {
            // pos      // uv
            -0.5f, -0.5f, 0.0f, 0.0f,   // bottom-left
             0.5f, -0.5f, 1.0f, 0.0f,   // bottom-right
             0.5f,  0.5f, 1.0f, 1.0f,   // top-right

            -0.5f, -0.5f, 0.0f, 0.0f,   // bottom-left
             0.5f,  0.5f, 1.0f, 1.0f,   // top-right
            -0.5f,  0.5f, 0.0f, 1.0f    // top-left
        };



        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        //glDeleteBuffers(1, &VBO); // VAO keeps the reference
    }
};
