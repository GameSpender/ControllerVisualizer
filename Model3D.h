#pragma once
#include "Transform3D.h"
#include "Services.h"
#include <memory>
#include <string>

// Forward-declare your model class (replace with actual model type)
class Model;

class Model3D : public Transform3D {
public:
    std::shared_ptr<Model> model;
    std::string name;

    Model3D() = default;

    // Accept const ref or by value
    Model3D(const std::string& n) {
        name = n;
        if (Services::assets) {
			Model* m = Services::assets->getModel(name);
            if (m) model = std::shared_ptr<Model>(m); // wrap pointer
        }
    }

    void setModel(const std::shared_ptr<Model>& m) { model = m; }
    std::shared_ptr<Model> getModel() const { return model; }

    void update(double dt) override {
        Transform3D::update(dt);
        // advance animations if needed
    }
};
