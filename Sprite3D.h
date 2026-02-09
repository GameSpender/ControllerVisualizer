#pragma once
#include "Transform3D.h"
#include "Services.h"
#include "AssetManager.h"
#include "Renderable.h"
#include <memory>
#include <string>

class Texture;

class Sprite3D : public Transform3D, public IRenderable {
public:
    enum class Mode {
        Normal,              // Use full transform
        Billboard,           // Full spherical billboard
        BillboardVertical    // Y-up cylindrical billboard
    };
    std::weak_ptr<Texture> texture;
    std::string name;

    Mode mode = Mode::Normal;

    Sprite3D() = default;

    Sprite3D(const std::string& textureName, Mode mode = Mode::Normal) {
        setTexture(textureName);
		this->mode = mode;
    }

    void setTexture(const std::string& textureName) {
        name = textureName;
        texture.reset();

        if (Services::assets) {
            texture = Services::assets->getTexture(name);
        }
    }

    void setEmissiveColor(const glm::vec3& emissive) {
        auto tex = getTexture().lock();
        if (tex) {
            tex->emissive = emissive;
        }
	}

    std::weak_ptr<Texture> getTexture() {
        if (texture.expired() && Services::assets) {
            texture = Services::assets->getTexture(name);
        }
        return texture;
    }

    void update(double dt) override {
        Transform3D::update(dt);
    }
};
