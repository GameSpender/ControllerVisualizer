#include "Physics.h"
#include "PhysicalActor2D.h"

//void PhysicsComponent::integrate(double dt, bool isKinematic) {
//	Actor2D& actor = static_cast<Actor2D&>(*owner);
//    actor.position += velocity * static_cast<float>(dt);
//    actor.rotation += angularVelocity * static_cast<float>(dt);
//
//    if (isKinematic) return;
//
//    if (friction > 0.0f)
//        velocity -= velocity * friction * static_cast<float>(dt);
//    if (angularFriction > 0.0f)
//        angularVelocity -= angularVelocity * angularFriction * static_cast<float>(dt);
//}

void PhysicsComponent::update(double dt) {
    Actor2D& actor = static_cast<Actor2D&>(*owner);
    actor.position += velocity * static_cast<float>(dt);
    actor.rotation += angularVelocity * static_cast<float>(dt);

    if (isKinematic) return;

    if (friction > 0.0f)
        velocity -= velocity * friction * static_cast<float>(dt);
    if (angularFriction > 0.0f)
        angularVelocity -= angularVelocity * angularFriction * static_cast<float>(dt);
}

void PhysicsComponent::applyForce(const glm::vec2& force, double dt) {
    velocity += force * static_cast<float>(dt) / mass;
}

void PhysicsComponent::applyTorque(float torque, double dt) {
    angularVelocity += torque * static_cast<float>(dt) / mass;
}

void PhysicsComponent::applyImpulse(const glm::vec2& impulse) {
    velocity += impulse / mass;
}

void PhysicsComponent::applyAngularImpulse(float impulse) {
    angularVelocity += impulse / mass;
}

void PhysicsComponent::reset() {
    velocity = glm::vec2(0.0f);
    angularVelocity = 0.0f;
}
