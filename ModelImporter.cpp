#include "ModelImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "stb_image.h"

Mesh::~Mesh() {
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

/* --------------------------
   Helpers
-------------------------- */
glm::mat4 ModelImporter::aiMatToGlm(const aiMatrix4x4& a) {
    return glm::mat4(
        a.a1, a.b1, a.c1, a.d1,
        a.a2, a.b2, a.c2, a.d2,
        a.a3, a.b3, a.c3, a.d3,
        a.a4, a.b4, a.c4, a.d4
    );
}

GLuint ModelImporter::loadEmbeddedTexture(const aiTexture* tex, const std::string& key) {
    GLuint texID = 0;
    if (!tex) return 0;

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    if (tex->mHeight == 0) {
        // Compressed image (PNG/JPG)
        int width, height, channels;
        unsigned char* data = stbi_load_from_memory(
            reinterpret_cast<const unsigned char*>(tex->pcData),
            tex->mWidth,
            &width, &height, &channels, 0
        );
        if (!data) {
            std::cerr << "Failed to load embedded texture " << key << std::endl;
            glDeleteTextures(1, &texID);
            return 0;
        }

        GLint format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else {
        // Raw RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->mWidth, tex->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pcData);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

GLuint ModelImporter::loadTextureFromMaterial(const aiMaterial* material, aiTextureType type, const aiScene* scene, Model& model) {
    if (!material) return 0;

    aiString path;
    if (material->GetTexture(type, 0, &path) != AI_SUCCESS) return 0;
    std::string key = path.C_Str();

    // Already loaded?
    if (model.textureCache.count(key)) return model.textureCache[key];

    GLuint texID = 0;

    if (!key.empty() && key[0] == '*') {
        int idx = std::stoi(key.substr(1));
        if (idx >= 0 && idx < (int)scene->mNumTextures) {
            texID = loadEmbeddedTexture(scene->mTextures[idx], key);
        }
    }
    else {
        // Optional: fallback for external files
        // texID = loadTextureFromFile(key);
    }

    if (texID) model.textureCache[key] = texID;
    return texID;
}

/* --------------------------
   Process a single mesh
-------------------------- */
std::unique_ptr<Mesh> ModelImporter::processMesh(const aiMesh* mesh, const aiScene* scene, Model& model) {
    auto outMesh = std::make_unique<Mesh>();

    std::vector<Vertex> vertices(mesh->mNumVertices);
    std::vector<uint32_t> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex v{};
        v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        if (mesh->HasNormals())
            v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        if (mesh->HasTextureCoords(0))
            v.texCoord = { mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y };
        if (mesh->HasTangentsAndBitangents())
            v.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
        vertices[i] = v;
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.push_back(mesh->mFaces[i].mIndices[j]);
    }

    outMesh->indexCount = static_cast<GLsizei>(indices.size());

    // Setup OpenGL buffers
    glGenVertexArrays(1, &outMesh->vao);
    glGenBuffers(1, &outMesh->vbo);
    glGenBuffers(1, &outMesh->ebo);

    glBindVertexArray(outMesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, outMesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outMesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glBindVertexArray(0);

    // -------------------------
    // Load material + textures
    // -------------------------
    if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
        const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        outMesh->material.albedoTexture = loadTextureFromMaterial(mat, aiTextureType_DIFFUSE, scene, model);
        outMesh->material.normalTexture = loadTextureFromMaterial(mat, aiTextureType_NORMALS, scene, model);
        outMesh->material.metallicRoughnessTexture = loadTextureFromMaterial(mat, aiTextureType_METALNESS, scene, model);
        outMesh->material.occlusionTexture = loadTextureFromMaterial(mat, aiTextureType_AMBIENT_OCCLUSION, scene, model);
        outMesh->material.emissiveTexture = loadTextureFromMaterial(mat, aiTextureType_EMISSIVE, scene, model);

        aiColor4D baseColor;
        if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor))
            outMesh->material.baseColor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
    }

    return outMesh;
}

/* --------------------------
   Recursive node traversal
-------------------------- */
void ModelImporter::processNode(const aiNode* node, const aiScene* scene, Model& model, const glm::mat4& parentTransform) {
    glm::mat4 nodeTransform = parentTransform * aiMatToGlm(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto m = processMesh(mesh, scene, model);
        m->transform = nodeTransform;
        model.meshes.push_back(std::move(m));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, model, nodeTransform);
}

/* --------------------------
   Main import function
-------------------------- */
std::unique_ptr<Model> ModelImporter::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace
    );

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Assimp load error: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    auto model = std::make_unique<Model>();
    processNode(scene->mRootNode, scene, *model, glm::mat4(1.0f));
    return model;
}
