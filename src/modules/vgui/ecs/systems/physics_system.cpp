//
// Created by floppyhammer on 4/2/2022.
//

#include "physics_system.h"

#include "../components/components.h"
#include "../coordinator.h"

namespace Pathfinder {
    void PhysicsSystem::update(float dt) {
        for (auto const &entity: entities) {
            auto coordinator = Coordinator::get_singleton();

            auto &rigidBody = coordinator.get_component<RigidBody>(entity);
            auto &transform = coordinator.get_component<Transform>(entity);
            auto const &gravity = coordinator.get_component<Gravity>(entity);

            //transform.position += rigidBody.velocity * dt;
            //rigidBody.velocity += gravity.force * dt;
        }
    }
}
