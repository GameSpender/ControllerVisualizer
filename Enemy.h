#pragma once
#include "Ship.h"
#include "Projectile.h"
#include "SpriteRenderer.h"
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>

using namespace glm;

class Enemy : public Transform2D, public Animated {
public:
    vec2 screenMin, screenMax;
    unsigned int texture;
    unsigned int projectileTexture;

    vec2 velocity = vec2(0.0f);
    float health = 25.0f;
    float maxSpeed = 100.0f;
    float rotationSpeed = 2.0f; // radians per second
    float shotInterval = 3.0f;  // seconds between shots
    float nextShot = 0.0f;
    float projectileSpeed = 300.0f;

    Enemy(unsigned int tex, unsigned projTex, int screenWidth, int screenHeight)
        : texture(tex), projectileTexture(projTex), screenMin(vec2(0.0f)), screenMax(vec2(screenWidth, screenHeight)) {
        nextShot = glfwGetTime() + linearRand(shotInterval * 0.2f, shotInterval);
    }

    void update(double dt) override {
        float delta = (float)dt;

        // Move
        position += velocity * delta;

        borderCollision();

        markDirty();
    }

    // Check if enemy is hit by a projectile
    bool checkHit(const Projectile& proj) {
        float dist = length(proj.position - position);
        return dist < length(getWorldScale()) * 0.6f; // example radius
    }

    bool canShoot() {
        if (!projectileTexture) return false;
        float time = glfwGetTime();
        return time > nextShot;

    }

    // Enemy fires a projectile
    Projectile shootAt(vec2 targetPos, vec2 targetSpeed, double dt) {       
       
        
       
        vec2 dir = normalize(targetPos + targetSpeed * (targetPos - position) / projectileSpeed - position);
        rotation = atan2(-dir.y, dir.x) - radians(90.0f);
        nextShot = glfwGetTime() + shotInterval;
        return Projectile(projectileTexture, position, dir * projectileSpeed, 6.0f, 30.0f, rotation);
    }

private:
    void borderCollision()
    {
        // Check X axis
        if (position.x < screenMin.x)
        {
            position.x = screenMin.x; // clamp to border
            if (velocity.x < 0)
            {
                velocity.x = -velocity.x;
            }
        }
        else if (position.x > screenMax.x)
        {
            position.x = screenMax.x;
            if (velocity.x > 0)
            {
                velocity.x = -velocity.x;
            }
        }

        // Check Y axis
        if (position.y < screenMin.y)
        {
            position.y = screenMin.y;
            if (velocity.y < 0)
            {
                velocity.y = -velocity.y;
            }
        }
        else if (position.y > screenMax.y)
        {
            position.y = screenMax.y;
            if (velocity.y > 0)
            {
                velocity.y = -velocity.y;
            }
        }
    }
};
