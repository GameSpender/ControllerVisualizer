#include "Physics.h"
#include "PhysicalActor2D.h"

void PhysicsComponent::integrate(PhysicalActor2D& actor, double dt, bool isKinematic) {
    actor.position += actor.velocity * static_cast<float>(dt);
    actor.rotation += actor.angularVelocity * static_cast<float>(dt);

    if (isKinematic) return;

    if (friction > 0.0f)
        actor.velocity -= actor.velocity * friction * static_cast<float>(dt);
    if (angularFriction > 0.0f)
        actor.angularVelocity -= actor.angularVelocity * angularFriction * static_cast<float>(dt);
}

void PhysicsComponent::applyForce(PhysicalActor2D& actor, const glm::vec2& force, double dt) {
    actor.velocity += force * static_cast<float>(dt) / mass;
}

void PhysicsComponent::applyTorque(PhysicalActor2D& actor, float torque, double dt) {
    actor.angularVelocity += torque * static_cast<float>(dt) / mass;
}

void PhysicsComponent::applyImpulse(PhysicalActor2D& actor, const glm::vec2& impulse) {
    actor.velocity += impulse / mass;
}

void PhysicsComponent::applyAngularImpulse(PhysicalActor2D& actor, float impulse) {
    actor.angularVelocity += impulse / mass;
}

void PhysicsComponent::reset(PhysicalActor2D& actor) {
    actor.velocity = glm::vec2(0.0f);
    actor.angularVelocity = 0.0f;
}
