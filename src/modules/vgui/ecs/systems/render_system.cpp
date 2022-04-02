//
// Created by floppyhammer on 4/2/2022.
//

#include "render_system.h"

#include "../components/components.h"
#include "../coordinator.h"

namespace Pathfinder {
    void RenderSystem::draw(uint32_t command_buffer) {
        for (auto const &entity: entities) {
            auto coordinator = Coordinator::get_singleton();
            //auto &texture = coordinator.GetComponent<Texture>(entity);

            // Draw call.
        }
    }
}
