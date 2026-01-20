#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ModelImporter.h" // Model + Mesh + Material

class ModelRenderer {
public:
    GLuint shader;

    ModelRenderer(GLuint shaderProgram)
        : shader(shaderProgram) {
    }

    // ------------------------------------
    // Draw a model with per-mesh transforms
    // ------------------------------------
    void Draw(const Model* model, const glm::mat4& rootTransform, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos) const {
        if (!model) return;

        glUseProgram(shader);

        // Set camera uniform
        glUniform3fv(glGetUniformLocation(shader, "uCameraPos"), 1, glm::value_ptr(cameraPos));

        for (const auto& mesh : model->meshes) {
            // -------------------------
            // Compute MVP with mesh transform
            // -------------------------
            glm::mat4 mvp = proj * view * rootTransform * mesh->transform;
            glUniformMatrix4fv(glGetUniformLocation(shader, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));

            // -------------------------
            // Bind textures
            // -------------------------

            glUniform1i(glGetUniformLocation(shader, "uUseTex"), GL_TRUE);
            bindTexture(mesh->material.albedoTexture, "uAlbedoTex", 0);
            bindTexture(mesh->material.metallicRoughnessTexture, "uMetallicRoughnessTex", 1);
            bindTexture(mesh->material.normalTexture, "uNormalTex", 2);
            bindTexture(mesh->material.occlusionTexture, "uOcclusionTex", 3);
            bindTexture(mesh->material.emissiveTexture, "uEmissiveTex", 4);

            // -------------------------
            // Set PBR uniforms
            // -------------------------
            glUniform4fv(glGetUniformLocation(shader, "uBaseColor"), 1, glm::value_ptr(mesh->material.baseColor));
            glUniform1f(glGetUniformLocation(shader, "uMetallic"), mesh->material.metallicFactor);
            glUniform1f(glGetUniformLocation(shader, "uRoughness"), mesh->material.roughnessFactor);
            glUniform3fv(glGetUniformLocation(shader, "uEmissive"), 1, glm::value_ptr(mesh->material.emissiveFactor));

            // -------------------------
            // Draw mesh
            // -------------------------
            glBindVertexArray(mesh->vao);
            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }

private:
    // -------------------------
    // Helper: bind texture if valid
    // -------------------------
    void bindTexture(GLuint tex, const char* uniformName, GLuint slot) const {
        if (tex == 0) return;
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(glGetUniformLocation(shader, uniformName), slot);
    }
};
