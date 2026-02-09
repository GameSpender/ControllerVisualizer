#pragma once
#include "Transform3D.h"
#include "Services.h"
#include "Renderable.h"
#include <memory>
#include <string>

// Forward-declare your model class (replace with actual model type)
class Model;

class Model3D : public Transform3D, public IRenderable {
public:
    std::weak_ptr<Model> model;
    std::string name;

    Model3D() = default;

    // Accept const ref or by value
    Model3D(const std::string& n) {
		setModel(n);
    }

    void setModel(const std::string& n) {
        name = n;
        model.reset(); // Clear current model
        if (Services::assets) {
            model = Services::assets->getModel(name);
        }
	}

    std::weak_ptr<Model> getModel() {
        if (model.expired() && Services::assets) {
            model = Services::assets->getModel(name);
        }
        return model;
	}

    void update(double dt) override {
        Transform3D::update(dt);
        // advance animations if needed
    }

    void applyTransform(const glm::mat4& transform, const std::string& meshName) {
        auto mdl = getModel().lock();
        if (!mdl) return;

        for (auto& mesh : mdl->meshes) {
            if (!mesh) continue;

            // If mesh name matches
            if (mesh->name == meshName) {
                // Apply the transform offset
                mesh->offset = transform;
            }
        }
    }

    std::weak_ptr<Mesh> findMesh(const std::string& meshName) {
        auto mdl = getModel().lock();
        if (!mdl) return std::weak_ptr<Mesh>();

        for (auto& mesh : mdl->meshes) {
            if (!mesh) continue;
            
            return mesh;
        }
        return std::weak_ptr<Mesh>();
    }

};
