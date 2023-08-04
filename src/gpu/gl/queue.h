#ifndef PATHFINDER_GPU_QUEUE_GL_H
#define PATHFINDER_GPU_QUEUE_GL_H

#include <vector>

#include "../queue.h"
#include "swap_chain.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class QueueGl : public Queue {
    friend class WindowGl;

public:
    void submit_and_wait(std::shared_ptr<CommandEncoder> encoder) override {
        if (encoder->submitted) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        encoder->finish();

        for (auto &callback : encoder->callbacks) {
            callback();
        }

        encoder->callbacks.clear();
    };

    void submit(std::shared_ptr<CommandEncoder> encoder, std::shared_ptr<SwapChain> surface) override {
        submit_and_wait(encoder);
    };
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_QUEUE_GL_H
