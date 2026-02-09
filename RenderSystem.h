#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "glm/glm.hpp"
#include "Renderable.h"
#include "ModelRenderer.h"
#include "SpriteRenderer.h"
#include <GL/glew.h>

class RenderSystem {
public:
    // store weak_ptrs so RenderSystem does not own objects
    std::vector<std::weak_ptr<IRenderable>> renderables;

    ModelRenderer* modelRenderer = nullptr;
    SpriteRenderer* spriteRenderer = nullptr;

    RenderSystem() = default;

    RenderSystem(ModelRenderer* mr, SpriteRenderer* sr)
        : modelRenderer(mr), spriteRenderer(sr) {
    }

    void submit(const std::shared_ptr<IRenderable>& obj) {
        if (obj)
            renderables.push_back(obj);
    }

    void clear() {
        renderables.clear();
    }

    void renderAll(const glm::mat4& view,
        const glm::mat4& proj,
        const glm::vec3& cameraPos)
    {
        // Filter expired pointers in-place
        renderables.erase(
            std::remove_if(renderables.begin(), renderables.end(),
                [](const std::weak_ptr<IRenderable>& wptr) {
                    return wptr.expired();
                }),
            renderables.end());

        // Render valid objects
        for (auto& wobj : renderables) {
            auto obj = wobj.lock();
            if (!obj) continue;

            // Try Model3D
            if (auto model = std::dynamic_pointer_cast<Model3D>(obj)) {
                if (modelRenderer)
                    modelRenderer->Draw(model, view, proj, cameraPos);
                continue;
            }

            // Disable depth for sprites
            glDisable(GL_DEPTH_TEST);

            // Try Sprite3D
            if (auto sprite = std::dynamic_pointer_cast<Sprite3D>(obj)) {
                if (spriteRenderer)
                    spriteRenderer->Draw(sprite, view, proj, cameraPos);
                continue;
            }

            // Future types can go here
        }
    }
};
