//
// Created by floppyhammer on 4/2/2022.
//

#ifndef PATHFINDER_ENTITY_MANAGER_H
#define PATHFINDER_ENTITY_MANAGER_H

#include "entity.h"
#include "component.h"

#include <cassert>
#include <queue>
#include <array>

namespace Pathfinder {
    class EntityManager {
    public:
        EntityManager() {
            // Initialize the queue with all possible entity IDs.
            for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
                available_entities.push(entity);
            }
        }

        Entity create_entity() {
            assert(living_entity_count < MAX_ENTITIES && "Too many entities in existence!");

            // Take an ID from the front of the queue.
            Entity id = available_entities.front();
            available_entities.pop();
            living_entity_count++;

            return id;
        }

        void destroy_entity(Entity entity) {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            // Invalidate the destroyed entity's signature
            signatures[entity].reset();

            // Put the destroyed ID at the back of the queue
            available_entities.push(entity);
            living_entity_count--;
        }

        void set_signature(Entity p_entity, Signature p_signature) {
            assert(p_entity < MAX_ENTITIES && "Entity out of range!");

            // Put this entity's signature into the array.
            signatures[p_entity] = p_signature;
        }

        Signature get_signature(Entity entity) {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            // Get this entity's signature from the array.
            return signatures[entity];
        }

    private:
        // Queue of unused entity IDs.
        std::queue<Entity> available_entities;

        // Array of signatures where the index corresponds to the entity ID.
        std::array<Signature, MAX_ENTITIES> signatures;

        // Total living entities - used to keep limits on how many exist.
        uint32_t living_entity_count = 0;
    };
}

#endif //PATHFINDER_ENTITY_MANAGER_H
