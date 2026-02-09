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


void ModelImporter::printMaterialDebug(const aiMaterial* mat) {
    if (!mat) return;

    std::cout << "=== Material Debug ===" << std::endl;

    // Print textures
    for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; ++type) {
        int count = mat->GetTextureCount(static_cast<aiTextureType>(type));
        if (count == 0) continue;

        std::cout << "Texture type " << type << " count: " << count << std::endl;
        for (int i = 0; i < count; ++i) {
            aiString path;
            if (mat->GetTexture(static_cast<aiTextureType>(type), i, &path) == AI_SUCCESS) {
                std::cout << "  Texture " << i << ": " << path.C_Str() << std::endl;
            }
        }
    }

    // Print diffuse/base color
    aiColor4D diff;
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, diff)) {
        std::cout << "Diffuse color: " << diff.r << ", " << diff.g << ", " << diff.b << ", " << diff.a << std::endl;
    }

    // Print emissive
    aiColor4D emi;
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_EMISSIVE, emi)) {
        std::cout << "Emissive color: " << emi.r << ", " << emi.g << ", " << emi.b << std::endl;
    }

    // Print metallic/roughness factors
    float metallic = 1.0f, roughness = 1.0f;
    mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
    std::cout << "Metallic: " << metallic << ", Roughness: " << roughness << std::endl;

    std::cout << "======================" << std::endl;
}


//GLuint ModelImporter::loadEmbeddedTexture(const aiTexture* tex, const std::string& key) {
//    GLuint texID = 0;
//    if (!tex) return 0;
//
//    glGenTextures(1, &texID);
//    glBindTexture(GL_TEXTURE_2D, texID);
//
//    if (tex->mHeight == 0) {
//        // Compressed image (PNG/JPG)
//        int width, height, channels;
//        unsigned char* data = stbi_load_from_memory(
//            reinterpret_cast<const unsigned char*>(tex->pcData),
//            tex->mWidth,
//            &width, &height, &channels, 0
//        );
//        if (!data) {
//            std::cerr << "Failed to load embedded texture " << key << std::endl;
//            glDeleteTextures(1, &texID);
//            return 0;
//        }
//
//        GLint format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
//        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//        stbi_image_free(data);
//    }
//    else {
//        // Raw RGBA
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->mWidth, tex->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pcData);
//    }
//
//    glGenerateMipmap(GL_TEXTURE_2D);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glBindTexture(GL_TEXTURE_2D, 0);
//
//    return texID;
//}


// Get texture key (just the filename) from material
std::string ModelImporter::getTextureKeyFromMaterial(
    const aiMaterial* material,
    aiTextureType type)
{
    if (!material) return "";

    aiString path;
    if (material->GetTexture(type, 0, &path) != AI_SUCCESS)
        return "";

    return std::string(path.C_Str()); // just the key/filename
}

std::shared_ptr<Mesh> ModelImporter::processMesh(
    const aiMesh* mesh,
    const aiScene* scene)
{
    auto outMesh = std::make_shared<Mesh>();

    outMesh->name = mesh->mName.C_Str();

    std::vector<Vertex> vertices(mesh->mNumVertices);
    std::vector<uint32_t> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex v{};
        v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        if (mesh->HasNormals())
            v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        if (mesh->HasTextureCoords(0))
            v.texCoord = { mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y };
        vertices[i] = v;
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.push_back(mesh->mFaces[i].mIndices[j]);
    }

    outMesh->indexCount = static_cast<GLsizei>(indices.size());

    // OpenGL buffer setup remains the same
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

    glBindVertexArray(0);

    // -------------------------
    // Material (store keys only)
    // -------------------------
    if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
        const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        printMaterialDebug(mat);

        outMesh->material.baseColorKey = getTextureKeyFromMaterial(mat, aiTextureType_DIFFUSE);
        outMesh->material.metallicKey = getTextureKeyFromMaterial(mat, aiTextureType_METALNESS);
        outMesh->material.roughnessKey = getTextureKeyFromMaterial(mat, aiTextureType_DIFFUSE_ROUGHNESS);
        outMesh->material.emissiveKey = getTextureKeyFromMaterial(mat, aiTextureType_EMISSIVE);

        float metallic = 1.0f;
        float roughness = 1.0f;
        aiColor4D color(1.f, 1.f, 1.f, 1.f);

        mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
        mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);

        outMesh->material.metallicFactor = metallic;
        outMesh->material.roughnessFactor = roughness;
		outMesh->material.baseColorFactor = glm::vec4(color.r, color.g, color.b, color.a);
		


        aiColor3D emissive(0.f, 0.f, 0.f);
        if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive))
            outMesh->material.emissiveFactor = glm::vec3(emissive.r, emissive.g, emissive.b);
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
        auto m = processMesh(mesh, scene);
        m->transform = nodeTransform;
        model.meshes.push_back(std::move(m));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, model, nodeTransform);
}

/* --------------------------
   Main import function
-------------------------- */
std::shared_ptr<Model> ModelImporter::loadModel(const std::string& path, const std::string& name) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path + name,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs
    );

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Assimp load error: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    auto model = std::make_shared<Model>();
    model.get()->directory = path;
    processNode(scene->mRootNode, scene, *model, glm::mat4(1.0f));
    return model;
}
