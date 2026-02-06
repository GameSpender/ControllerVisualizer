#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

#include "Util.h"
#include "ModelImporter.h"


class Texture {
public:
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

class AssetManager {
public:
    void loadTexture(const std::string& name, const std::string& filePath) {
        if (textures.find(name) != textures.end()) return;

        std::unique_ptr<Texture> tex = std::make_unique<Texture>();
        tex->id = preprocessTexture(filePath.c_str());
        std::cout << "Loading texture: " << filePath << std::endl;
        textures[name] = std::move(tex);
    }

    Texture* getTexture(const std::string& name) {
        auto it = textures.find(name);
        if (it != textures.end()) return it->second.get();
        std::cerr << "Texture not found: " << name << std::endl;
        return nullptr;
    }

    // --------------------
    // Models
    // --------------------
    void loadModel(const std::string& name, const std::string& filePath) {
        if (models.find(name) != models.end()) return;

		std::string directory = filePath.substr(0, filePath.find_last_of("/\\") + 1);
		std::string filename = filePath.substr(filePath.find_last_of("/\\") + 1);
		std::cout << "Loading model: " << directory + filename << std::endl;

        auto model = ModelImporter::loadModel(directory, filename);
        if (!model) {
            std::cerr << "Failed to load model: " << filePath << std::endl;
            return;
        }

        for (const auto& mesh : model->meshes) {
            // Preload all textures used by the model
            if (!mesh->material.baseColorKey.empty())
                loadTexture(mesh->material.baseColorKey, directory + mesh->material.baseColorKey);
            if (!mesh->material.metallicKey.empty())
                loadTexture(mesh->material.metallicKey, directory + mesh->material.metallicKey);
            if (!mesh->material.roughnessKey.empty())
                loadTexture(mesh->material.roughnessKey, directory + mesh->material.roughnessKey);
            if (!mesh->material.emissiveKey.empty())
                loadTexture(mesh->material.emissiveKey, directory + mesh->material.emissiveKey);
		}

        std::cout << "Loaded model: " << filePath << std::endl;
        models[name] = std::move(model);
    }

    Model* getModel(const std::string& name) {
        auto it = models.find(name);
        if (it != models.end()) return it->second.get();
        std::cerr << "Model not found: " << name << std::endl;
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
    std::unordered_map<std::string, std::unique_ptr<Model>> models;
};
