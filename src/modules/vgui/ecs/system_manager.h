//
// Created by floppyhammer on 4/2/2022.
//

#ifndef PATHFINDER_SYSTEM_MANAGER_H
#define PATHFINDER_SYSTEM_MANAGER_H

#include "entity.h"
#include "component.h"
#include "system.h"

#include <memory>
#include <unordered_map>
#include <cassert>

namespace Pathfinder {
    class SystemManager {
    public:
        template<typename T>
        std::shared_ptr<T> register_system() {
            const char *typeName = typeid(T).name();

            assert(systems.find(typeName) == systems.end() && "Registering system more than once.");

            // Create a pointer to the system and return it so it can be used externally
            auto system = std::make_shared<T>();
            systems.insert({typeName, system});
            return system;
        }

        template<typename T>
        void set_signature(Signature signature) {
            const char *typeName = typeid(T).name();

            assert(systems.find(typeName) != systems.end() && "System used before registered.");

            // Set the signature for this system
            signatures.insert({typeName, signature});
        }

        void entity_destroyed(Entity entity) {
            // Erase a destroyed entity from all system lists
            // mEntities is a set so no check needed
            for (auto const &pair: systems) {
                auto const &system = pair.second;

                system->entities.erase(entity);
            }
        }

        void entity_signature_changed(Entity entity, Signature entitySignature) {
            // Notify each system that an entity's signature changed
            for (auto const &pair: systems) {
                auto const &type = pair.first;
                auto const &system = pair.second;
                auto const &systemSignature = signatures[type];

                // Entity signature matches system signature - insert into set
                if ((entitySignature & systemSignature) == systemSignature) {
                    system->entities.insert(entity);
                }
                    // Entity signature does not match system signature - erase from set
                else {
                    system->entities.erase(entity);
                }
            }
        }

    private:
        // Map from system type string pointer to a signature
        std::unordered_map<const char *, Signature> signatures{};

        // Map from system type string pointer to a system pointer
        std::unordered_map<const char *, std::shared_ptr<System>> systems{};
    };
}

#endif //PATHFINDER_SYSTEM_MANAGER_H
