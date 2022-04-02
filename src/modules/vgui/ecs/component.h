//
// Created by floppyhammer on 4/2/2022.
//

#ifndef PATHFINDER_COMPONENT_H
#define PATHFINDER_COMPONENT_H

#include "../../../common/math/vec3.h"
#include "../../../common/math/quaternion.h"
#include "entity.h"

#include <bitset>
#include <unordered_map>
#include <cassert>

namespace Pathfinder {
    // A simple type alias
    using ComponentType = std::uint8_t;

    // Used to define the size of arrays later on
    const ComponentType MAX_COMPONENTS = 32;

    // A simple type alias
    using Signature = std::bitset<MAX_COMPONENTS>;

    class Component {

    };

    struct Transform {
        Vec3<float> position;
        Quaternion rotation;
        Vec3<float> scale;
    };

    /// The virtual inheritance of IComponentArray is unfortunate but, as far as I can tell, unavoidable.
    /// As seen later, we'll have a list of every ComponentArray (one per component type), and we need to
    /// notify all of them when an entity is destroyed so that it can remove the entity's data if it exists.
    /// The only way to keep a list of multiple templated types is to keep a list of their common interface
    /// so that we can call entity_destroyed() on all of them.
    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;

        virtual void entity_destroyed(Entity entity) = 0;
    };

    template<typename T>
    class ComponentArray : public IComponentArray {
    public:
        void insert_data(Entity entity, T component) {
            assert(entity_to_index_map.find(entity) == entity_to_index_map.end() &&
                   "Component added to same entity more than once.");

            // Put new entry at end and update the maps
            size_t new_index = size;
            entity_to_index_map[entity] = new_index;
            index_to_entity_map[new_index] = entity;
            component_array[new_index] = component;
            ++size;
        }

        void remove_data(Entity entity) {
            assert(entity_to_index_map.find(entity) != entity_to_index_map.end() && "Removing non-existent component.");

            // Copy element at end into deleted element's place to maintain density
            size_t indexOfRemovedEntity = entity_to_index_map[entity];
            size_t indexOfLastElement = size - 1;
            component_array[indexOfRemovedEntity] = component_array[indexOfLastElement];

            // Update map to point to moved spot
            Entity entityOfLastElement = index_to_entity_map[indexOfLastElement];
            entity_to_index_map[entityOfLastElement] = indexOfRemovedEntity;
            index_to_entity_map[indexOfRemovedEntity] = entityOfLastElement;

            entity_to_index_map.erase(entity);
            index_to_entity_map.erase(indexOfLastElement);

            --size;
        }

        T &get_data(Entity entity) {
            assert(entity_to_index_map.find(entity) != entity_to_index_map.end() && "Retrieving non-existent component.");

            // Return a reference to the entity's component
            return component_array[entity_to_index_map[entity]];
        }

        void entity_destroyed(Entity entity) override {
            if (entity_to_index_map.find(entity) != entity_to_index_map.end()) {
                // Remove the entity's component if it existed
                remove_data(entity);
            }
        }

    private:
        // The packed array of components (of generic type T),
        // set to a specified maximum amount, matching the maximum number
        // of entities allowed to exist simultaneously, so that each entity
        // has a unique spot.
        std::array<T, MAX_ENTITIES> component_array;

        // Map from an entity ID to an array index.
        std::unordered_map<Entity, size_t> entity_to_index_map;

        // Map from an array index to an entity ID.
        std::unordered_map<size_t, Entity> index_to_entity_map;

        // Total size of valid entries in the array.
        size_t size;
    };
}

#endif //PATHFINDER_COMPONENT_H
