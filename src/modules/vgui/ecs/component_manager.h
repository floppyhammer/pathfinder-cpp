//
// Created by floppyhammer on 4/2/2022.
//

#ifndef PATHFINDER_COMPONENT_MANAGER_H
#define PATHFINDER_COMPONENT_MANAGER_H

#include "component.h"
#include "entity.h"

#include <memory>

namespace Pathfinder {
    /**
     * Component Manager is in charge of talking to all of the
     * different ComponentArrays when a component needs to be
     * added or removed.
     */
    class ComponentManager {
    public:
        template<typename T>
        void register_component() {
            const char *typeName = typeid(T).name();

            assert(component_types.find(typeName) == component_types.end() &&
                   "Registering component type more than once.");

            // Add this component type to the component type map
            component_types.insert({typeName, next_component_type});

            // Create a ComponentArray pointer and add it to the component arrays map
            component_arrays.insert({typeName, std::make_shared<ComponentArray<T >>()});

            // Increment the value so that the next component registered will be different
            ++next_component_type;
        }

        template<typename T>
        ComponentType get_component_type() {
            const char *typeName = typeid(T).name();

            assert(component_types.find(typeName) != component_types.end() && "Component not registered before use.");

            // Return this component's type - used for creating signatures
            return component_types[typeName];
        }

        template<typename T>
        void add_component(Entity entity, T component) {
            // Add a component to the array for an entity
            get_component_array<T>()->InsertData(entity, component);
        }

        template<typename T>
        void remove_component(Entity entity) {
            // Remove a component from the array for an entity
            get_component_array<T>()->RemoveData(entity);
        }

        template<typename T>
        T &get_component(Entity entity) {
            // Get a reference to a component from the array for an entity
            return get_component_array<T>()->GetData(entity);
        }

        void entity_destroyed(Entity entity) {
            // Notify each component array that an entity has been destroyed
            // If it has a component for that entity, it will remove it
            for (auto const &pair: component_arrays) {
                auto const &component = pair.second;

                component->entity_destroyed(entity);
            }
        }

    private:
        // Map from type string pointer to a component type
        std::unordered_map<const char *, ComponentType> component_types{};

        // Map from type string pointer to a component array
        std::unordered_map<const char *, std::shared_ptr<IComponentArray>> component_arrays{};

        // The component type to be assigned to the next registered component - starting at 0
        ComponentType next_component_type{};

        // Convenience function to get the statically cast pointer to the ComponentArray of type T.
        template<typename T>
        std::shared_ptr<ComponentArray<T>> get_component_array() {
            const char *typeName = typeid(T).name();

            assert(component_types.find(typeName) != component_types.end() && "Component not registered before use.");

            return std::static_pointer_cast<ComponentArray<T >>(component_arrays[typeName]);
        }
    };
}

#endif //PATHFINDER_COMPONENT_MANAGER_H
