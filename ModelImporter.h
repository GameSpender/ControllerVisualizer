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
};


struct Material {
    std::string baseColorKey;
    std::string metallicKey;
    std::string roughnessKey;
    std::string emissiveKey;

    glm::vec4 baseColorFactor = { 1,1,1,1 };
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
    glm::mat4 offset = glm::mat4(1.0f);

    std::string name;
    ~Mesh();
};

class Model {
public:
    std::vector<std::shared_ptr<Mesh>> meshes;
	std::string directory; // Directory of the model file for resolving external textures
};

/* --------------------------
   Model Importer
-------------------------- */
class ModelImporter {
public:
    // Load a model from file (.glb/.gltf/.obj etc.)
    static std::shared_ptr<Model> loadModel(const std::string& path, const std::string& name);
	static void printMaterialDebug(const aiMaterial* material);

private:
    // Recursive node traversal
    static void processNode(const aiNode* node, const aiScene* scene, Model& model, const glm::mat4& parentTransform);

    // Process a single mesh
    static std::shared_ptr<Mesh> processMesh(const aiMesh* mesh, const aiScene* scene);

    // Load a texture from material
    static std::string getTextureKeyFromMaterial(const aiMaterial* material, aiTextureType type);

    // Load a texture directly from an embedded aiTexture
    //static GLuint loadEmbeddedTexture(const aiTexture* texture, const std::string& key);

    // Convert Assimp matrix to glm::mat4
    static glm::mat4 aiMatToGlm(const aiMatrix4x4& mat);
};
