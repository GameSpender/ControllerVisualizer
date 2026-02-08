#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Services.h" // For texture loading
#include "AssetManager.h"
#include "LightManager.h"
#include "Model3D.h"

class ModelRenderer {
public:
    GLuint shader;

    ModelRenderer(GLuint shaderProgram)
        : shader(shaderProgram) {
    }

    // ------------------------------------
    // Draw a model with per-mesh transforms
    // ------------------------------------
    void Draw(const Model* model, const glm::mat4& rootTransform, const glm::mat4& view,
        const glm::mat4& proj, const glm::vec3& cameraPos) const
    {
        if (!model) return;

        glUseProgram(shader);

        // Set camera uniform
        glUniform3fv(glGetUniformLocation(shader, "uCameraPos"), 1, glm::value_ptr(cameraPos));
        
        vec3 ambientColor = vec3(0.01f);
        if (Services::lights) {
            ambientColor = Services::lights->ambientColor;
        }

        // Set ambient light uniform
        glUniform3fv(glGetUniformLocation(shader, "uAmbientColor"), 1, glm::value_ptr(ambientColor));

        // Before drawing meshes:
		auto activeLights = Services::lights->getActiveLights();
        int numPointLights = 0;

        for (int i = 0; i < activeLights.size(); ++i) {
            auto pl = std::dynamic_pointer_cast<PointLight2D>(activeLights[i]);
            if (!pl) continue; // Skip if not a PointLight
            if (numPointLights >= 50) continue;

            std::string prefix = "uPointLights[" + std::to_string(i) + "].";

            glUniform3fv(glGetUniformLocation(shader, (prefix + "position").c_str()), 1, glm::value_ptr(pl->position));
            glUniform3fv(glGetUniformLocation(shader, (prefix + "color").c_str()), 1, glm::value_ptr(pl->color));
            glUniform1f(glGetUniformLocation(shader, (prefix + "intensity").c_str()), pl->intensity);
            glUniform1f(glGetUniformLocation(shader, (prefix + "range").c_str()), pl->range);
            glUniform1f(glGetUniformLocation(shader, (prefix + "falloff").c_str()), pl->falloff);


            numPointLights++;
        }

        glUniform1i(glGetUniformLocation(shader, "uNumPointLights"), numPointLights);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        for (const auto& mesh : model->meshes)
        {

            glm::mat4 modelMatrix = rootTransform * mesh->transform;
            glm::mat4 mvp = proj * view * modelMatrix;

            glUniformMatrix4fv(glGetUniformLocation(shader, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
            glUniformMatrix4fv(glGetUniformLocation(shader, "uModel"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

            // --- Bind base color texture if available ---
            glUniform1i(glGetUniformLocation(shader, "uHasBaseColorTex"),
                !mesh->material.baseColorKey.empty() ? 1 : 0);
            if (!mesh->material.baseColorKey.empty()) {
                bindTexture(mesh->material.baseColorKey, model->directory, "uBaseColorTex", 0);
            }
            else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1i(glGetUniformLocation(shader, "uBaseColorTex"), 0);   
            }

            // --- Set scalar factors ---
            glUniform4fv(glGetUniformLocation(shader, "uBaseColorFactor"), 1,
                glm::value_ptr(mesh->material.baseColorFactor));

            glUniform1f(glGetUniformLocation(shader, "uMetallicFactor"),
                mesh->material.metallicFactor);

            glUniform1f(glGetUniformLocation(shader, "uRoughnessFactor"),
                mesh->material.roughnessFactor);

            glUniform3fv(glGetUniformLocation(shader, "uEmissiveFactor"), 1,
                glm::value_ptr(mesh->material.emissiveFactor));

            glBindVertexArray(mesh->vao);

            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }

    // -------------------- New overload for Model3D --------------------
    void Draw(std::shared_ptr<Model3D> node, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos) const {
        if (!node) return;
        Draw(node->getModel().lock().get(), node->getWorldMatrix(), view, proj, cameraPos);
    }

private:
    void bindTexture(const std::string& key, const std::string& modelDirectory, const char* uniformName, GLuint slot) const {
        if (key.empty()) return; // No texture, nothing to bind

        std::string fullPath = modelDirectory + key;

        std::weak_ptr<Texture> tex = Services::assets->getTexture(key);
        if (tex.expired()) {
            Services::assets->loadTexture(key, fullPath);
            tex = Services::assets->getTexture(key);
        }

        if (tex.expired()) return;

        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, tex.lock()->id);
        glUniform1i(glGetUniformLocation(shader, uniformName), slot);
    }
};
