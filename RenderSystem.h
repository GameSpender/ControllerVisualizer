#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "glm/glm.hpp"
#include "Renderable.h"
#include "ModelRenderer.h"
#include "SpriteRenderer.h"

class RenderSystem {
public:
    std::vector<std::shared_ptr<IRenderable>> renderables;

    ModelRenderer* modelRenderer = nullptr;
    SpriteRenderer* spriteRenderer = nullptr;

	RenderSystem() = default;

    RenderSystem(ModelRenderer* mr, SpriteRenderer* sr)
		: modelRenderer(mr), spriteRenderer(sr) {
	}

    void submit(const std::shared_ptr<IRenderable>& obj) {
        renderables.push_back(obj);
    }

    void clear() {
        renderables.clear();
    }

    void renderAll(const glm::mat4& view,
        const glm::mat4& proj,
        const glm::vec3& cameraPos)
    {
        for (auto& obj : renderables)
        {
            if (!obj) continue;

            // Try Model3D
            if (auto model = std::dynamic_pointer_cast<Model3D>(obj))
            {
                if (modelRenderer)
                    modelRenderer->Draw(model, view, proj, cameraPos);
                continue;
            }

            // Try Sprite3D
            if (auto sprite = std::dynamic_pointer_cast<Sprite3D>(obj))
            {
                if (spriteRenderer)
                    spriteRenderer->Draw(sprite, view, proj, cameraPos);
                continue;
            }

            // Future types can go here
        }
    }
};
