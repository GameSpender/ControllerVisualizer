#pragma once
#include "Transform3D.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

class UpdateSystem {
public:
    // SYSTEM OWNS all nodes
    std::vector<std::shared_ptr<Transform3D>> nodes;
    std::vector<std::shared_ptr<Transform3D>> pendingAdd;

    // Add a node to be managed
    void addNode(const std::shared_ptr<Transform3D>& node) {
        if (!node) return;
        pendingAdd.push_back(node);
    }

    // Update all nodes
    void update(double dt) {
        // Merge pending additions
        if (!pendingAdd.empty()) {
            nodes.insert(nodes.end(), pendingAdd.begin(), pendingAdd.end());
            pendingAdd.clear();
        }

        // --- UPDATE PHASE ---
        for (auto& node : nodes) {
            if (!node->deleted) {
                node->update(dt);
            }
        }

        // --- CLEANUP PHASE ---
        // Remove deleted nodes after update
        nodes.erase(
            std::remove_if(nodes.begin(), nodes.end(),
                [this](const std::shared_ptr<Transform3D>& node) {
                    if (node->deleted) {
                        // Unhook from parent if still attached
                        auto parent = node->parent.lock();
                        if (parent) {
                            parent->removeChild(node);
                        }

                        return true; // remove from system
                    }
                    return false;
                }),
            nodes.end());
    }

    // Remove a node from its parent manually
    void removeParent(const std::shared_ptr<Transform3D>& node) {
        if (!node) return;
        auto parent = node->parent.lock();
        if (parent) {
            parent->removeChild(node);
        }
    }
};
