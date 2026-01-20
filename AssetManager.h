#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

#include "Util.h"


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

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
};
