#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "Actor.h"
#include "PhysicalActor2D.h"
#include "SimpleShootingAi.h"
#include <glm/glm.hpp>

class Director {
public:
    std::vector<Actor2D*> opponents;              // raw opponent pointers
    std::vector<SimpleShootingAi*> activeAIs;         // active AI controllers

    void addOpponent(Actor2D* actor) {
        if (!actor) return;
        if (std::find(opponents.begin(), opponents.end(), actor) == opponents.end())
            opponents.push_back(actor);
    }

    void removeOpponent(Actor2D* actor) {
        opponents.erase(std::remove(opponents.begin(), opponents.end(), actor), opponents.end());
    }

    void addAI(SimpleShootingAi* ai) {
        if (!ai) return;
        if (std::find(activeAIs.begin(), activeAIs.end(), ai) == activeAIs.end())
            activeAIs.push_back(ai);
    }

    void removeAI(SimpleShootingAi* ai) {
        activeAIs.erase(std::remove(activeAIs.begin(), activeAIs.end(), ai), activeAIs.end());
    }

    // Update all AIs, providing each with the closest opponent's position & velocity
    void update(double dt) {
        if (opponents.empty() || activeAIs.empty()) return;

        for (auto* ai : activeAIs) {
            if (!ai) continue;

            Actor2D* closestOpponent = nullptr;
            float minDistSqr = std::numeric_limits<float>::max();

            for (auto* opp : opponents) {
                glm::vec2 diff = opp->getWorldPosition() - ai->pawnPosition();
                float distSqr = glm::dot(diff, diff);
                if (distSqr < minDistSqr) {
                    minDistSqr = distSqr;
                    closestOpponent = opp;
                }
            }

            if (closestOpponent) {
                // Try to cast to PhysicalActor2D to get velocity, otherwise default to zero
                glm::vec2 targetVel(0.0f);
                if (auto phys = dynamic_cast<PhysicalActor2D*>(closestOpponent)) {
                    targetVel = phys->getVelocity();
                }

                ai->setTarget(closestOpponent->getWorldPosition(), targetVel);
                ai->update(dt);
            }
        }
    }
};
