#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <assimp/scene.h>

/* --------------------------
   Engine-side types
-------------------------- */
struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texCoord{};
    glm::vec3 tangent{};
};

struct Material {
    GLuint albedoTexture = 0;
    GLuint normalTexture = 0;
    GLuint metallicRoughnessTexture = 0;
    GLuint occlusionTexture = 0;
    GLuint emissiveTexture = 0;

    glm::vec4 baseColor = { 1,1,1,1 };
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec3 emissiveFactor = { 0,0,0 };
};

class Mesh {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
    Material material;
    glm::mat4 transform = glm::mat4(1.0f);

    ~Mesh();
};

class Model {
public:
    std::vector<std::unique_ptr<Mesh>> meshes;
    // Texture cache for embedded textures
    std::unordered_map<std::string, GLuint> textureCache;
};

/* --------------------------
   Model Importer
-------------------------- */
class ModelImporter {
public:
    // Load a model from file (.glb/.gltf/.obj etc.)
    static std::unique_ptr<Model> loadModel(const std::string& path);

private:
    // Recursive node traversal
    static void processNode(const aiNode* node, const aiScene* scene, Model& model, const glm::mat4& parentTransform);

    // Process a single mesh
    static std::unique_ptr<Mesh> processMesh(const aiMesh* mesh, const aiScene* scene, Model& model);

    // Load a texture from material (handles embedded GLB textures)
    static GLuint loadTextureFromMaterial(const aiMaterial* material, aiTextureType type, const aiScene* scene, Model& model);

    // Load a texture directly from an embedded aiTexture
    static GLuint loadEmbeddedTexture(const aiTexture* texture, const std::string& key);

    // Convert Assimp matrix to glm::mat4
    static glm::mat4 aiMatToGlm(const aiMatrix4x4& mat);
};
